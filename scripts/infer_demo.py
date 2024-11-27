from typing import List

import cv2
import librosa
import numpy as np
import onnxruntime as ort
import torch
import torchaudio.compliance.kaldi as kaldi


def audio2feature(audio: str, wenet_encoder_weight: str) -> np.ndarray:

    waveform = audio.astype("float32") * 32767.
    waveform = waveform.astype("int16")
    empty_audio_30 = np.zeros([32*160], dtype="int16")
    empty_audio_31 = np.zeros([35*160], dtype="int16")
    waveform = np.concatenate(
        [empty_audio_30, waveform, empty_audio_31], axis=0)

    waveform = torch.from_numpy(waveform).float().unsqueeze(0)
    fbank_feature = kaldi.fbank(waveform,
                                num_mel_bins=80,
                                frame_length=25,
                                frame_shift=10,
                                dither=0.0,
                                energy_floor=0.0,
                                sample_frequency=16000)
    fbank_feature = fbank_feature.unsqueeze(0)
    fbank_feature = fbank_feature.detach().cpu().numpy()
    fbank_feature_length = int(fbank_feature.shape[1])

    wenet_model = ort.InferenceSession(
        wenet_encoder_weight,
        providers=["CUDAExecutionProvider", "CPUExecutionProvider"]
    )

    offset = np.ones((1, ), dtype=np.int64)*100
    att_cache = np.zeros([3, 8, 16, 128], dtype=np.float32)
    cnn_cache = np.zeros([3, 1, 512, 14], dtype=np.float32)

    wenet_feature = []
    frames_stride = 67
    start = 0
    end = 0
    while end < fbank_feature_length:
        end = start + frames_stride

        tmp_feat = fbank_feature[:, start:end, :]
        if tmp_feat.shape[1] < frames_stride:
            zero_pad = np.zeros([1, frames_stride-tmp_feat.shape[1], 80])
            tmp_feat = np.concatenate((tmp_feat, zero_pad), axis=1)
        chunk_feat = np.expand_dims(tmp_feat, axis=0)
        start += 5

        inputs = {
            "chunk": chunk_feat.astype("float32"),
            "offset": offset,
            "att_cache": att_cache.astype("float32"),
            "cnn_cache": cnn_cache.astype("float32")
        }
        outputs = wenet_model.run(None, inputs)
        y = outputs[0][0]
        wenet_feature.append(y)

    wenet_feature = np.array(wenet_feature, dtype="float32")
    return wenet_feature


def get_sliced_feature(feature: np.ndarray, frame_idx: int) -> np.ndarray:

    left = frame_idx - 8
    right = frame_idx + 8
    pad_left = 0
    pad_right = 0
    if left < 0:
        pad_left = -left
        left = 0
    if right > feature.shape[0]:
        pad_right = right - feature.shape[0]
        right = feature.shape[0]
    auds = feature[left:right].copy()
    if pad_left > 0:
        auds = np.concatenate(
            [np.zeros_like(auds[:pad_left]), auds], axis=0)
    if pad_right > 0:
        auds = np.concatenate(
            [auds, np.zeros_like(auds[:pad_right])], axis=0)
    return auds


def feature2chunks(feature_array: np.ndarray) -> List[np.ndarray]:

    audio_chunks = []
    for i in range(feature_array.shape[0]):
        selected_feature = get_sliced_feature(feature_array, i)
        audio_chunks.append(selected_feature)

    return audio_chunks


def print_feature_stats(wenet_features):
    print("=== Python WenetFeatures Statistics ===")
    print(f"Total features: {len(wenet_features)}")

    # 打印第一个特征的统计信息
    first_feature = wenet_features[0]
    print(f"\nFirst feature shape: {first_feature.shape}")
    print(f"First feature first 5 values: {first_feature[0, :5]}")  # 第一行前5个值
    print(f"First feature mean: {np.mean(first_feature):.6f}")
    print(f"First feature std: {np.std(first_feature):.6f}")
    print(f"First feature min: {np.min(first_feature):.6f}")
    print(f"First feature max: {np.max(first_feature):.6f}")

    # 打印最后一个特征的统计信息
    middle_feature = wenet_features[-2]
    print(f"\nMiddle feature shape: {middle_feature.shape}")
    print(f"Middle feature first 5 values: {middle_feature[0, :5]}")
    print(f"Middle feature mean: {np.mean(middle_feature):.6f}")
    print(f"Middle feature std: {np.std(middle_feature):.6f}")
    print(f"Middle feature min: {np.min(middle_feature):.6f}")
    print(f"Middle feature max: {np.max(middle_feature):.6f}")


if __name__ == "__main__":
    wenet_weight = "/home/sinter/workspace/lip-sync-cpp/tests/models/wenet_encoder.onnx"
    wav2lip_weight = "/home/sinter/workspace/lip-sync-cpp/tests/models/w2l_with_wenet.onnx"
    input_size, pad_size = 160, 4

    audio_path = "/home/sinter/workspace/lip-sync-cpp/tests/data/test.wav"
    image_path = "/home/sinter/workspace/lip-sync-cpp/tests/data/image.jpg"
    face_bbox = (476, 832, 645, 1001)
    use_half = False
    device = "cpu"

    # initial wav2lip model
    wav2lip_model = ort.InferenceSession(
        wav2lip_weight, providers=["CPUExecutionProvider"])

    # load audio
    audio, _ = librosa.load(audio_path, sr=16000)

    # audio feature extraction
    audio_feature = audio2feature(audio, wenet_weight)
    print_feature_stats(audio_feature)

    audio_chunks = feature2chunks(audio_feature)

    # load image/video
    frame = cv2.imread(image_path, 1)
    # face cropping
    x1, y1, x2, y2 = face_bbox
    face_crop_large = frame[y1:y2, x1:x2].copy()

    # pre-processing
    face_crop_large = cv2.resize(
        face_crop_large,
        (input_size+pad_size*2, input_size+pad_size*2),
        interpolation=cv2.INTER_LINEAR
    )
    face_crop = face_crop_large[
        pad_size:input_size+pad_size,
        pad_size:input_size+pad_size, :
    ].copy()
    face_mask = face_crop.copy()
    face_mask = cv2.rectangle(
        face_mask,
        (pad_size+1, pad_size+1, input_size-pad_size*2-2, input_size-pad_size*3-3),
        color=(0, 0, 0), thickness=-1
    )
    face_crop = face_crop.transpose(2, 0, 1).astype("float32")
    face_mask = face_mask.transpose(2, 0, 1).astype("float32")
    face_crop = face_crop[np.newaxis, ...] / 255.
    face_mask = face_mask[np.newaxis, ...] / 255.
    x = np.concatenate([face_crop, face_mask], axis=1)
    a = audio_chunks[0].reshape(256, 16, 32)[np.newaxis, ...]
    a = a.astype("float32")

    # inference
    pred = wav2lip_model.run(
        ["output"],
        {"image": x, "audio": a}
    )[0]

    # post-processing
    pred = pred.transpose(0, 2, 3, 1) * 255.
    pred = pred[0].astype("uint8")
    face_crop_large[
        pad_size:input_size+pad_size,
        pad_size:input_size+pad_size, :
    ] = pred
    face_crop_large = cv2.resize(
        face_crop_large, (x2 - x1, y2 - y1),
        interpolation=cv2.INTER_LINEAR
    )
    frame_out = frame.copy()
    frame_out[y1:y2, x1:x2] = face_crop_large

    cv2.imwrite("show.png", frame_out)
