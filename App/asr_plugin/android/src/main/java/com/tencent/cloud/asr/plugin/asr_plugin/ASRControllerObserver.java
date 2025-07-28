package com.tencent.cloud.asr.plugin.asr_plugin;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.tencent.aai.audio.utils.WavCache;
import com.tencent.aai.exception.ClientException;
import com.tencent.aai.exception.ServerException;
import com.tencent.aai.listener.AudioRecognizeResultListener;
import com.tencent.aai.listener.AudioRecognizeStateListener;
import com.tencent.aai.model.AudioRecognizeRequest;
import com.tencent.aai.model.AudioRecognizeResult;

import java.io.DataOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.Map;

import io.flutter.plugin.common.MethodChannel;

public class ASRControllerObserver implements AudioRecognizeStateListener, AudioRecognizeResultListener {

    private int _id = 0;
    private MethodChannel _channel;
    private RandomAccessFile _stream = null;
    private String _path = "";

    ASRControllerObserver(int id, MethodChannel channel){
        _id = id;
        _channel = channel;
    }

    public void setAudioFile(String path) {
        try {
            _path = path;
            _stream = new RandomAccessFile(path, "rw");
            ByteBuffer buffer = ByteBuffer.allocate(4);
            buffer.order(ByteOrder.LITTLE_ENDIAN);
            _stream.write(new byte[]{'R', 'I', 'F', 'F'});
            _stream.writeInt(0);
            _stream.write(new byte[]{'W', 'A', 'V', 'E'});
            _stream.write(new byte[]{'f', 'm', 't', ' '});
            buffer.putInt(0, 16);
            _stream.write(buffer.array());
            buffer.putShort(0, (short) 1);
            _stream.write(buffer.array(), 0, 2);
            _stream.write(buffer.array(), 0, 2);
            buffer.putInt(0, 16000);
            _stream.write(buffer.array());
            buffer.putInt(0, 16000*2);
            _stream.write(buffer.array());
            buffer.putShort(0, (short) 2);
            _stream.write(buffer.array(), 0, 2);
            buffer.putShort(0, (short) 16);
            _stream.write(buffer.array(), 0, 2);
            _stream.write(new byte[]{'d', 'a', 't', 'a'});
            _stream.writeInt(0);
        } catch (IOException e) {
            e.printStackTrace();
            _stream = null;
            new Handler(Looper.getMainLooper()).post(new Runnable() {
                @Override
                public void run() {
                    Map<String, Object> args = new HashMap<>();
                    args.put("id", _id);
                    args.put("code", -1);
                    args.put("message", e.getMessage());
                    _channel.invokeMethod("onAudioFile", args);
                }
            });
        }
    }

    @Override
    public void onSliceSuccess(AudioRecognizeRequest request, AudioRecognizeResult result, int order) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                Map<String, Object> args = new HashMap<>();
                args.put("id", _id);
                args.put("sentence_id", order);
                args.put("sentence_text", result.getText());
                _channel.invokeMethod("onSliceSuccess", args);
            }
        });
    }

    @Override
    public void onSegmentSuccess(AudioRecognizeRequest request, AudioRecognizeResult result, int order) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                Map<String, Object> args = new HashMap<>();
                args.put("id", _id);
                args.put("sentence_id", order);
                args.put("sentence_text", result.getText());
                _channel.invokeMethod("onSegmentSuccess", args);
            }
        });
    }

    @Override
    public void onSuccess(AudioRecognizeRequest request, String result) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                // Log.d("asr_plugin", "call onSuccess from Android");
                Map<String, Object> args = new HashMap<>();
                args.put("id", _id);
                args.put("text", result);
                // Log.d("asr_plugin", String.format("%d %s", _id, result));
                // Log.d("asr_plugin", "invoke onSuccess start");
                _channel.invokeMethod("onSuccess", args);
                // Log.d("asr_plugin", "invoke onSuccess end");
            }
        });
    }

    @Override
    public void onFailure(AudioRecognizeRequest request, ClientException clientException, ServerException serverException, String response) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                Map<String, Object> args = new HashMap<>();
                args.put("id", _id);
                if(clientException != null) {
                    args.put("code", clientException.getCode());
                    args.put("message", clientException.getMessage());
                }else{
                    args.put("code", -1);
                    args.put("message", serverException.toString());
                }
                args.put("response", response);
                _channel.invokeMethod("onFailed", args);
            }
        });
    }


    @Override
    public void onStartRecord(AudioRecognizeRequest request) {
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                Map<String, Object> args = new HashMap<>();
                args.put("id", _id);
                _channel.invokeMethod("onStartRecord", args);
            }
        });
    }

    @Override
    public void onStopRecord(AudioRecognizeRequest request) {
        if(_stream != null) {
            try {
                ByteBuffer buffer = ByteBuffer.allocate(4);
                buffer.order(ByteOrder.LITTLE_ENDIAN);
                int len = (int) _stream.getFilePointer();
                _stream.seek(4);
                buffer.putInt(0, len);
                _stream.write(buffer.array());
                _stream.seek(40);
                buffer.putInt(0, len - 36);
                _stream.write(buffer.array());
                _stream.close();
                new Handler(Looper.getMainLooper()).post(new Runnable() {
                    @Override
                    public void run() {
                        Map<String, Object> args = new HashMap<>();
                        args.put("id", _id);
                        args.put("code", 0);
                        args.put("message", _path);
                        _channel.invokeMethod("onAudioFile", args);
                    }
                });
            } catch (IOException e) {
                e.printStackTrace();
                new Handler(Looper.getMainLooper()).post(new Runnable() {
                    @Override
                    public void run() {
                        Map<String, Object> args = new HashMap<>();
                        args.put("id", _id);
                        args.put("code", -1);
                        args.put("message", e.getMessage());
                        _channel.invokeMethod("onAudioFile", args);
                    }
                });
            } finally {
                _stream = null;
            }
        }
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                Map<String, Object> args = new HashMap<>();
                args.put("id", _id);
                _channel.invokeMethod("onStopRecord", args);
            }
        });
    }

    @Override
    public void onVoiceVolume(AudioRecognizeRequest request, int volume) {

    }

    @Override
    public void onNextAudioData(short[] audioDatas, int readBufferLength) {
        if (_stream == null) {
            return;
        }
        ByteBuffer buffer = ByteBuffer.allocate(2);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        for (int i = 0; i < audioDatas.length; i++) {
            try {
                buffer.putShort(0, audioDatas[i]);
                _stream.write(buffer.array());
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void onSilentDetectTimeOut() {

    }

    @Override
    public void onVoiceDb(float val) {

    }
}
