package com.tencent.cloud.asr.plugin.asr_plugin;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Build;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;

import com.tencent.aai.AAIClient;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Random;

import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.embedding.engine.plugins.activity.ActivityAware;
import io.flutter.embedding.engine.plugins.activity.ActivityPluginBinding;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.MethodChannel.MethodCallHandler;
import io.flutter.plugin.common.MethodChannel.Result;

/** AsrPlugin */
public class AsrPlugin implements FlutterPlugin, MethodCallHandler, ActivityAware {
  /// The MethodChannel that will the communication between Flutter and native Android
  ///
  /// This local reference serves to register the plugin with the Flutter Engine and unregister it
  /// when the Flutter Engine is detached from the Activity
  private MethodChannel channel;
  private ArrayList<Object> instance_mgr = new ArrayList<>();
  @SuppressLint("StaticFieldLeak")
  private static Activity activity;

  @Override
  public void onAttachedToEngine(@NonNull FlutterPluginBinding flutterPluginBinding) {
    channel = new MethodChannel(flutterPluginBinding.getBinaryMessenger(), "asr_plugin");
    channel.setMethodCallHandler(this);
  }

  @Override
  public void onMethodCall(@NonNull MethodCall call, @NonNull Result result) {
    if (call.method.equals("ASRController.new")) {
      ASRControllerConfig config = new ASRControllerConfig();
      config.appID = call.argument("appID");
      config.secretID = call.argument("secretID");
      config.secretKey = call.argument("secretKey");
      config.token = call.argument("token");
      config.engine_model_type = call.argument("engine_model_type");
      config.filter_dirty = call.argument("filter_dirty");
      config.filter_modal = call.argument("filter_modal");
      config.filter_punc = call.argument("filter_punc");
      config.convert_num_mode = call.argument("convert_num_mode");
      config.hotword_id = call.argument("hotword_id");
      config.customization_id = call.argument("customization_id");
      config.vad_silence_time = call.argument("vad_silence_time");
      config.needvad = call.argument("needvad");
      config.word_info = call.argument("word_info");
      config.reinforce_hotword = call.argument("reinforce_hotword");
      config.noise_threshold = ((Double)call.argument("noise_threshold")).floatValue();

      config.is_compress = call.argument("is_compress");
      config.silence_detect = call.argument("silence_detect");
      config.silence_detect_duration = call.argument("silence_detect_duration");
      config.is_save_audio_file = call.argument("is_save_audio_file");
      config.audio_file_path = call.argument("audio_file_path");
      
      try {
        ASRController obj = new ASRController(activity, config);
        instance_mgr.add(obj);
        int id = instance_mgr.size() - 1;
        result.success(id);
      } catch (Exception e) {
        e.printStackTrace();
        result.error(e.toString(), e.toString(), e);
      }
    }
    else if(call.method.equals("ASRController.setObserver")){
      ASRController controller = (ASRController) instance_mgr.get(call.argument("id"));
      if(controller == null){
        result.error("No Instance", "No Instance", "No Instance");
        return;
      }
      int observer_id = call.argument("observer_id");
      controller.setObserver(new ASRControllerObserver(observer_id, channel));
      result.success(null);
    }
    else if(call.method.equals("ASRController.setDataSource")){
      ASRController controller = (ASRController) instance_mgr.get(call.argument("id"));
      if(controller == null){
        result.error("No Instance", "No Instance", "No Instance");
        return;
      }
      int datasource_id = call.argument("datasource_id");
      controller.setDataSource(new ASRControllerDataSource(datasource_id, channel));
      result.success(null);
    }
    else if(call.method.equals("ASRController.start")){
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
        if(activity.checkSelfPermission(Manifest.permission.RECORD_AUDIO) != PackageManager.PERMISSION_GRANTED) {
          activity.requestPermissions(new String[]{Manifest.permission.RECORD_AUDIO}, 1);
          result.error("没有权限", "", "");
          return;
        }
      }
      ASRController controller = (ASRController) instance_mgr.get(call.argument("id"));
      if(controller == null){
        result.error("No Instance", "No Instance", "No Instance");
        return;
      }
      controller.start();
      result.success(null);
    }
    else if(call.method.equals("ASRController.cancel")){
      ASRController controller = (ASRController) instance_mgr.get(call.argument("id"));
      if(controller == null){
        result.error("No Instance", "No Instance", "No Instance");
        return;
      }
      controller.cancel();
      result.success(null);
    }
    else if(call.method.equals("ASRController.stop")){
      ASRController controller = (ASRController) instance_mgr.get(call.argument("id"));
      if(controller == null){
        result.error("No Instance", "No Instance", "No Instance");
        return;
      }
      controller.stop();
      result.success(null);
    }
    else if(call.method.equals("ASRController.release")){
      int id = call.argument("id");
      ASRController controller = (ASRController) instance_mgr.get(id);
      if(controller == null){
        result.error("No Instance", "No Instance", "No Instance");
        return;
      }
      controller.stop();
      instance_mgr.set(id, null);
      result.success(null);
    }
    else {
      result.notImplemented();
    }
  }

  @Override
  public void onDetachedFromEngine(@NonNull FlutterPluginBinding binding) {
    channel.setMethodCallHandler(null);
  }

  @Override
  public void onAttachedToActivity(@NonNull ActivityPluginBinding binding) {
    activity = binding.getActivity();
    Log.e("LeeLog", "onAttachedToActivity: " + activity.toString());
  }

  @Override
  public void onDetachedFromActivityForConfigChanges() {

  }

  @Override
  public void onReattachedToActivityForConfigChanges(@NonNull ActivityPluginBinding binding) {
    activity = binding.getActivity();
    Log.e("LeeLog", "onReattachedToActivityForConfigChanges: " + activity.toString());
  }

  @Override
  public void onDetachedFromActivity() {

  }
}
