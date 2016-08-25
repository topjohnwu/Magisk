package com.topjohnwu.magisk;

import android.Manifest;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.support.annotation.IdRes;
import android.support.annotation.NonNull;
import android.support.design.widget.NavigationView;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.Html;
import android.text.TextUtils;
import android.view.MenuItem;
import android.view.View;
import android.widget.Toast;

import com.topjohnwu.magisk.utils.Shell;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;

public class WelcomeActivity extends AppCompatActivity implements NavigationView.OnNavigationItemSelectedListener {

    private static final String SELECTED_ITEM_ID = "SELECTED_ITEM_ID";
    private static final String JSON_UPDATE_CHECK = "https://raw.githubusercontent.com/topjohnwu/MagiskManager/updater/app/magisk_update.json";

    private final Handler mDrawerHandler = new Handler();

    @BindView(R.id.toolbar) Toolbar toolbar;
    @BindView(R.id.drawer_layout) DrawerLayout drawer;
    @BindView(R.id.nav_view) NavigationView navigationView;

    @IdRes
    private int mSelectedId = R.id.magisk;

    private String mLastLink;
    private boolean mLastIsApp;

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_welcome);
        ButterKnife.bind(this);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
        }

        // Load mods in the background
        ModulesFragment.loadMod = new ModulesFragment.loadModules();
        ModulesFragment.loadMod.execute();

        setSupportActionBar(toolbar);

        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close) {
            @Override
            public void onDrawerOpened(View drawerView) {
                super.onDrawerOpened(drawerView);
                super.onDrawerSlide(drawerView, 0); // this disables the arrow @ completed state
            }

            @Override
            public void onDrawerSlide(View drawerView, float slideOffset) {
                super.onDrawerSlide(drawerView, 0); // this disables the animation
            }
        };

        drawer.addDrawerListener(toggle);
        toggle.syncState();

        //noinspection ResourceType
        mSelectedId = savedInstanceState == null ? mSelectedId : savedInstanceState.getInt(SELECTED_ITEM_ID);
        navigationView.setCheckedItem(mSelectedId);

        if (savedInstanceState == null) {
            mDrawerHandler.removeCallbacksAndMessages(null);
            mDrawerHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    navigate(mSelectedId);
                }
            }, 250);
        }

        navigationView.setNavigationItemSelectedListener(this);

        new CheckUpdates(this).execute();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);

        outState.putInt(SELECTED_ITEM_ID, mSelectedId);
    }

    @Override
    public void onBackPressed() {
        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        } else {
            super.onBackPressed();
        }
    }

    @Override
    public boolean onNavigationItemSelected(@NonNull final MenuItem menuItem) {
        mSelectedId = menuItem.getItemId();
        mDrawerHandler.removeCallbacksAndMessages(null);
        mDrawerHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                navigate(menuItem.getItemId());
            }
        }, 250);

        drawer.closeDrawer(GravityCompat.START);
        return true;
    }

    private void navigate(final int itemId) {
        Fragment navFragment = null;
        String tag = "";
        switch (itemId) {
            case R.id.magisk:
                setTitle(R.string.magisk);
                tag = "magisk";
                navFragment = new MagiskFragment();
                break;
            case R.id.modules:
                setTitle(R.string.modules);
                tag = "modules";
                navFragment = new ModulesFragment();
                break;
            case R.id.log:
                setTitle(R.string.log);
                tag = "log";
                navFragment = new LogFragment();
                break;
            case R.id.app_about:
                startActivity(new Intent(this, AboutActivity.class));
                return;
        }

        if (navFragment != null) {
            FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
            transaction.setCustomAnimations(R.anim.fade_in, R.anim.fade_out);
            try {
                toolbar.setElevation(navFragment instanceof ModulesFragment ? 0 : 10);

                transaction.replace(R.id.content_frame, navFragment, tag).commit();
            } catch (IllegalStateException ignored) {
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if (requestCode == 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            new DownloadFile(this, mLastLink, mLastIsApp).execute();
        } else {
            Toast.makeText(this, R.string.permissionNotGranted, Toast.LENGTH_LONG).show();
        }
    }

    private class CheckUpdates extends AsyncTask<Void, Void, String> {

        private final Context context;

        public CheckUpdates(Context context) {
            this.context = context;
        }

        @Override
        protected String doInBackground(Void... voids) {
            try {
                HttpURLConnection c = (HttpURLConnection) new URL(JSON_UPDATE_CHECK).openConnection();
                c.setRequestMethod("GET");
                c.setInstanceFollowRedirects(false);
                c.setDoOutput(false);
                c.connect();

                BufferedReader br = new BufferedReader(new InputStreamReader(c.getInputStream()));
                StringBuilder sb = new StringBuilder();
                String line;
                while ((line = br.readLine()) != null) {
                    sb.append(line);
                }
                br.close();
                return sb.toString();
            } catch (IOException e) {
                return null;
            }
        }

        @Override
        protected void onPostExecute(String result) {
            super.onPostExecute(result);

            if (result == null) return;

            try {
                JSONObject json = new JSONObject(result);

                JSONObject app = json.getJSONObject("app");
                JSONObject magisk = json.getJSONObject("magisk");

                String appVersionCode = app.getString("versionCode");
                String appLink = app.getString("link");
                String appChangelog = app.getString("changelog");

                String magiskVersionCode = magisk.getString("versionCode");
                String magiskLink = magisk.getString("link");
                String magiskChangelog = magisk.getString("changelog");

                if (Integer.parseInt(appVersionCode) > BuildConfig.VERSION_CODE) {
                    showUpdateDialog(true, appVersionCode, appLink, appChangelog);
                }

                List<String> versionSh = Shell.sh("getprop magisk.version");

                String version = versionSh.isEmpty() ? "" : versionSh.get(0);

                int versionInt = TextUtils.isEmpty(version) ? 0 : Integer.parseInt(version);

                if (Integer.parseInt(magiskVersionCode) > versionInt) {
                    showUpdateDialog(false, magiskVersionCode, magiskLink, magiskChangelog);
                }

            } catch (JSONException ignored) {
            }
        }

        private boolean isUpdateIgnored(String version) {
            SharedPreferences prefs = getSharedPreferences(getPackageName() + "_preferences", MODE_PRIVATE);
            return prefs.getBoolean("update_ignored_" + version, false);
        }

        private void setUpdateIgnored(String version) {
            SharedPreferences prefs = getSharedPreferences(getPackageName() + "_preferences", MODE_PRIVATE);
            prefs.edit().putBoolean("update_ignored_" + version, true).apply();
        }

        private void showUpdateDialog(final boolean app, final String versionCode, final String link, String changelog) {
            if (isUpdateIgnored(versionCode)) return;

            String text = app ? getString(R.string.app_name) : getString(R.string.magisk);
            String msg = getString(R.string.update_available_message, text, versionCode, changelog);

            new AlertDialog.Builder(context)
                    .setTitle(R.string.update_available)
                    .setMessage(Html.fromHtml(msg))
                    .setCancelable(false)
                    .setPositiveButton(R.string.update, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialogInterface, int i) {
                            mLastLink = link;
                            mLastIsApp = app;

                            if (ActivityCompat.checkSelfPermission(WelcomeActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
                                    && Build.VERSION.SDK_INT >= 23) {
                                requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0);
                                return;
                            }

                            new DownloadFile(context, link, app).execute();
                        }
                    })
                    .setNegativeButton(R.string.no_thanks, null)
                    .setNeutralButton(R.string.never_show_again, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialogInterface, int i) {
                            setUpdateIgnored(versionCode);
                        }
                    })
                    .show();
        }
    }

    private class DownloadFile extends AsyncTask<Void, Integer, Boolean> {

        private final Context context;
        private final String link;
        private final File downloadFile;
        private final ProgressDialog progress;

        public DownloadFile(Context context, String link, boolean apk) {
            this.link = link;
            this.context = context;

            File dir = new File(Environment.getExternalStorageDirectory() + "/Magisk");

            if (!dir.exists()) dir.mkdir();

            if (apk) {
                downloadFile = new File(dir + "/MagiskManager.apk");
            } else {
                downloadFile = new File(dir + "/Magisk.zip");
            }

            Toast.makeText(context, downloadFile.getPath(), Toast.LENGTH_SHORT).show();

            progress = ProgressDialog.show(context, null, getString(R.string.loading), true, false);
        }

        @Override
        protected Boolean doInBackground(Void... voids) {
            try {
                URL u = new URL(link);
                URLConnection conn = u.openConnection();
                conn.connect();

                int length = conn.getContentLength();

                InputStream input = new BufferedInputStream(u.openStream(), 8192);
                OutputStream output = new FileOutputStream(downloadFile);

                byte data[] = new byte[1024];
                long total = 0;

                int count;
                while ((count = input.read(data)) != -1) {
                    total += count;
                    output.write(data, 0, count);

                    publishProgress((int) ((total * 100) / length));
                }

                output.flush();

                output.close();
                input.close();

                return true;
            } catch (IOException e) {
                return false;
            }
        }

        @Override
        protected void onProgressUpdate(Integer... values) {
            super.onProgressUpdate(values);

            progress.setMessage(getString(R.string.loading) + values[0] + "%");
        }

        @Override
        protected void onPostExecute(Boolean result) {
            super.onPostExecute(result);
            progress.dismiss();
            if (!result) {
                Toast.makeText(context, R.string.error_download_file, Toast.LENGTH_LONG).show();
                return;
            }

            if (downloadFile.getPath().contains("apk")) {
                Intent intent = new Intent(Intent.ACTION_VIEW);
                intent.setDataAndType(Uri.fromFile(downloadFile), "application/vnd.android.package-archive");
                intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
            } else {
                Toast.makeText(context, R.string.flash_recovery, Toast.LENGTH_LONG).show();
            }

        }
    }
}
