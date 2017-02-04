package com.topjohnwu.magisk;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import com.topjohnwu.magisk.utils.Async;

public class SplashActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        // Start all async tasks
        new Async.InitConfigs(getApplicationContext()){
            @Override
            protected void onPostExecute(Void v) {
                // Start main activity only after configs are loaded
                Intent intent = new Intent(getApplicationContext(), MainActivity.class);
                startActivity(intent);
                finish();
            }
        }.exec();
        new Async.GetBootBlocks().exec();
        new Async.CheckUpdates().exec();
        new Async.LoadModules() {
            @Override
            protected void onPostExecute(Void v) {
                super.onPostExecute(v);
                new Async.LoadRepos(getApplicationContext()).exec();
            }
        }.exec();
        new Async.LoadApps(getPackageManager()).exec();
    }
}
