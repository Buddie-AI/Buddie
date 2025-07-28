package com.tencent.cloud.asr.plugin.asr_plugin;

import android.os.Handler;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.tencent.aai.audio.data.PcmAudioDataSource;
import com.tencent.aai.exception.ClientException;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CountDownLatch;

import io.flutter.plugin.common.MethodChannel;

public class ASRControllerDataSource implements PcmAudioDataSource {
    private int _id = 0;
    private MethodChannel _channel;

    ASRControllerDataSource(int id, MethodChannel channel){
        _id = id;
        _channel = channel;
    }

    @Override
    public int read(short[] audioPcmData, int length) {
        Map<String, Object> args = new HashMap<>();
        args.put("id", _id);
        args.put("size", length * 2);
        CountDownLatch count = new CountDownLatch(1);
        final int[] real_len = {0};
        new Handler(Looper.getMainLooper()).post(new Runnable() {
            @Override
            public void run() {
                _channel.invokeMethod("read", args, new MethodChannel.Result() {
                    @Override
                    public void success(@Nullable Object result) {
                        byte[] content = (byte[]) result;
                        if (content != null && content.length != 0) {
                            ByteBuffer.wrap(content).order(ByteOrder.LITTLE_ENDIAN).asShortBuffer().get(audioPcmData);
                        }
                        real_len[0] = content.length / 2;
                        count.countDown();
                    }

                    @Override
                    public void error(@NonNull String errorCode, @Nullable String errorMessage, @Nullable Object errorDetails) {
                        count.countDown();
                    }

                    @Override
                    public void notImplemented() {
                        throw new RuntimeException();
                    }
                });
            }
        });
        try {
            count.await();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        return real_len[0];
    }

    @Override
    public void start() throws ClientException {

    }

    @Override
    public void stop() {

    }

    @Override
    public boolean isSetSaveAudioRecordFiles() {
        return false;
    }
}
