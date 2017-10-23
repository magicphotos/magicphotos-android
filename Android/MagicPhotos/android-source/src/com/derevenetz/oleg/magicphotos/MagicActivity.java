package com.derevenetz.oleg.magicphotos;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedOutputStream;
import java.io.FileOutputStream;

import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import org.qtproject.qt5.android.bindings.QtActivity;

import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.MobileAds;

public class MagicActivity extends QtActivity
{
    private static final int      REQUEST_CODE_SHOW_GALLERY    = 1001;

    private static final String   ADMOB_APP_ID                 = "ca-app-pub-2455088855015693~7279538773";
    private static final String   ADMOB_ADVIEW_UNIT_ID         = "ca-app-pub-2455088855015693/1811713388";
    private static final String   ADMOB_INTERSTITIALAD_UNIT_ID = "ca-app-pub-2455088855015693/6662028461";
    private static final String   ADMOB_TEST_DEVICE_ID         = "217F8CE186D0849DE13B0AFB1B1D7B3E";

    private static final AdSize   ADMOB_ADVIEW_ADSIZE          = AdSize.FLUID;

    private static boolean        statusBarVisible             = false,
                                  mobileAdsInitialized         = false;
    private static int            statusBarHeight              = 0;
    private static MagicActivity  instance                     = null;
    private static AdView         adView                       = null;
    private static InterstitialAd interstitialAd               = null;

    private static native void adViewHeightUpdated(int adview_height);

    private static native void imageSelected(String image_file, int image_orientation);
    private static native void imageSelectionCancelled();
    private static native void imageSelectionFailed();

    public MagicActivity()
    {
        instance = this;
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        int resource_id = getResources().getIdentifier("status_bar_height", "dimen", "android");

        if (resource_id > 0) {
            statusBarHeight = getResources().getDimensionPixelSize(resource_id);
        }

        if ((getWindow().getDecorView().getSystemUiVisibility() & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
            statusBarVisible = true;
        } else {
            statusBarVisible = false;
        }

        getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener() {
            @Override
            public void onSystemUiVisibilityChange(int visibility)
            {
                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                    statusBarVisible = true;

                    if (adView != null) {
                        int ad_visibility = adView.getVisibility();

                        adView.setVisibility(View.GONE);
                        adView.setY(statusBarHeight);
                        adView.setVisibility(ad_visibility);
                    }
                } else {
                    statusBarVisible = false;

                    if (adView != null) {
                        int ad_visibility = adView.getVisibility();

                        adView.setVisibility(View.GONE);
                        adView.setY(0);
                        adView.setVisibility(ad_visibility);
                    }
                }
            }
        });

        if (!mobileAdsInitialized) {
            MobileAds.initialize(this, ADMOB_APP_ID);

            mobileAdsInitialized = true;
        }
    }

    @Override
    public void onResume()
    {
        super.onResume();

        if (adView != null) {
            adView.resume();
        }
    }

    @Override
    public void onPause()
    {
        if (adView != null) {
            adView.pause();
        }

        super.onPause();
    }

    @Override
    public void onDestroy()
    {
        if (adView != null) {
            adView.destroy();

            adView = null;
        }

        super.onDestroy();
    }

    public static String getSaveDirectory()
    {
        File save_dir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).getAbsolutePath() + "/MagicPhotos");

        if (save_dir.mkdirs() || save_dir.isDirectory()) {
            return save_dir.getAbsolutePath();
        } else {
            return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).getAbsolutePath();
        }
    }

    public static int getScreenDPI()
    {
        DisplayMetrics metrics = instance.getResources().getDisplayMetrics();

        return metrics.densityDpi;
    }

    public static void showGallery()
    {
        try {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);

            intent.setType("image/*");

            instance.startActivityForResult(intent, REQUEST_CODE_SHOW_GALLERY);
        } catch (Exception ex) {
            imageSelectionFailed();
        }
    }

    public static void refreshGallery(String image_file)
    {
        MediaScannerConnection.scanFile(instance, new String[] { image_file }, null, null);
    }

    public static void shareImage(String image_file)
    {
        try {
            Intent intent = new Intent(Intent.ACTION_SEND);

            intent.setType("image/*");
            intent.putExtra(Intent.EXTRA_STREAM, Uri.fromFile(new File(image_file)));

            instance.startActivity(Intent.createChooser(intent, instance.getString(R.string.activity_header_share_image)));
        } catch (Exception ex) {
            // Ignore
        }
    }

    public static void showAdView()
    {
        instance.runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                View view = instance.getWindow().getDecorView().getRootView();

                if (view instanceof ViewGroup) {
                    ViewGroup view_group = (ViewGroup)view;

                    if (adView != null) {
                        view_group.removeView(adView);

                        adView.destroy();

                        adViewHeightUpdated(0);

                        adView = null;
                    }

                    FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT,
                                                                                   FrameLayout.LayoutParams.WRAP_CONTENT,
                                                                                   Gravity.CENTER_HORIZONTAL);

                    adView = new AdView(instance);

                    adView.setAdSize(ADMOB_ADVIEW_ADSIZE);
                    adView.setAdUnitId(ADMOB_ADVIEW_UNIT_ID);
                    adView.setLayoutParams(params);
                    adView.setVisibility(View.GONE);

                    if (statusBarVisible) {
                        adView.setY(statusBarHeight);
                    } else {
                        adView.setY(0);
                    }

                    adView.setAdListener(new AdListener() {
                        @Override
                        public void onAdLoaded()
                        {
                            if (adView != null) {
                                adView.setVisibility(View.VISIBLE);

                                adView.post(new Runnable() {
                                    @Override
                                    public void run()
                                    {
                                        if (adView != null) {
                                            adViewHeightUpdated(adView.getHeight());
                                        }
                                    }
                                });
                            }
                        }

                        @Override
                        public void onAdFailedToLoad(int errorCode)
                        {
                            if (adView != null) {
                                adView.setVisibility(View.VISIBLE);

                                adView.post(new Runnable() {
                                    @Override
                                    public void run()
                                    {
                                        if (adView != null) {
                                            adViewHeightUpdated(adView.getHeight());
                                        }
                                    }
                                });
                            }
                        }
                    });

                    view_group.addView(adView);

                    AdRequest.Builder builder = new AdRequest.Builder();

                    if (!ADMOB_TEST_DEVICE_ID.equals("")) {
                        builder.addTestDevice(ADMOB_TEST_DEVICE_ID);
                    }

                    adView.loadAd(builder.build());
                }
            }
        });
    }

    public static void hideAdView()
    {
        instance.runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                View view = instance.getWindow().getDecorView().getRootView();

                if (view instanceof ViewGroup) {
                    ViewGroup view_group = (ViewGroup)view;

                    if (adView != null) {
                        view_group.removeView(adView);

                        adView.destroy();

                        adViewHeightUpdated(0);

                        adView = null;
                    }
                }
            }
        });
    }

    public static void createInterstitialAd()
    {
        instance.runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                if (interstitialAd == null) {
                    interstitialAd = new InterstitialAd(instance);

                    interstitialAd.setAdUnitId(ADMOB_INTERSTITIALAD_UNIT_ID);

                    interstitialAd.setAdListener(new AdListener() {
                        @Override
                        public void onAdClosed()
                        {
                            if (interstitialAd != null) {
                                AdRequest.Builder builder = new AdRequest.Builder();

                                if (!ADMOB_TEST_DEVICE_ID.equals("")) {
                                    builder.addTestDevice(ADMOB_TEST_DEVICE_ID);
                                }

                                interstitialAd.loadAd(builder.build());
                            }
                        }

                        @Override
                        public void onAdFailedToLoad(int errorCode)
                        {
                            if (interstitialAd != null) {
                                new Handler().postDelayed(new Runnable() {
                                    @Override
                                    public void run()
                                    {
                                        if (interstitialAd != null) {
                                            AdRequest.Builder builder = new AdRequest.Builder();

                                            if (!ADMOB_TEST_DEVICE_ID.equals("")) {
                                                builder.addTestDevice(ADMOB_TEST_DEVICE_ID);
                                            }

                                            interstitialAd.loadAd(builder.build());
                                        }
                                    }
                                }, 60000);
                            }
                        }
                    });

                    AdRequest.Builder builder = new AdRequest.Builder();

                    if (!ADMOB_TEST_DEVICE_ID.equals("")) {
                        builder.addTestDevice(ADMOB_TEST_DEVICE_ID);
                    }

                    interstitialAd.loadAd(builder.build());
                }
            }
        });
    }

    public static void showInterstitialAd()
    {
        instance.runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                if (interstitialAd != null && interstitialAd.isLoaded()) {
                    interstitialAd.show();
                }
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == REQUEST_CODE_SHOW_GALLERY) {
            if (resultCode == RESULT_OK && data != null) {
                final Uri image_uri = data.getData();

                if (image_uri != null) {
                    (new Thread(new Runnable() {
                        @Override
                        public void run()
                        {
                            InputStream      input_stream       = null;
                            FileOutputStream file_output_stream = null;
                            OutputStream     output_stream      = null;

                            try {
                                input_stream = getContentResolver().openInputStream(image_uri);

                                Bitmap bitmap = BitmapFactory.decodeStream(input_stream);

                                if (bitmap != null) {
                                    final File cache_file = new File(getApplicationContext().getCacheDir().getAbsolutePath() + "/cache.jpg");

                                    if (cache_file.getParentFile().mkdirs() || cache_file.getParentFile().isDirectory()) {
                                        file_output_stream = new FileOutputStream(cache_file);
                                        output_stream      = new BufferedOutputStream(file_output_stream);

                                        bitmap.compress(Bitmap.CompressFormat.JPEG, 90, output_stream);

                                        output_stream.flush();
                                        output_stream.close();

                                        file_output_stream.flush();
                                        file_output_stream.close();

                                        String[] query_columns     = { MediaStore.Images.Media.ORIENTATION };
                                        String   image_orientation = null;
                                        Cursor   cursor            = null;

                                        try {
                                            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT && DocumentsContract.isDocumentUri(getApplicationContext(), image_uri)) {
                                                String id = DocumentsContract.getDocumentId(image_uri).split(":")[1];

                                                cursor = getContentResolver().query(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, query_columns, MediaStore.Images.Media._ID + " = ?", new String[]{id}, null);
                                            } else {
                                                cursor = getContentResolver().query(image_uri, query_columns, null, null, null);
                                            }

                                            if (cursor != null && cursor.moveToFirst()) {
                                                image_orientation = cursor.getString(cursor.getColumnIndex(query_columns[0]));
                                            }
                                        } catch (Exception ex) {
                                            // Ignore
                                        } finally {
                                            if (cursor != null) {
                                                cursor.close();
                                            }
                                        }

                                        final String final_image_orientation = image_orientation;

                                        runOnUiThread(new Runnable() {
                                            @Override
                                            public void run()
                                            {
                                                if (final_image_orientation == null) {
                                                    imageSelected(cache_file.getAbsolutePath(), 0);
                                                } else if (final_image_orientation.equals("0")) {
                                                    imageSelected(cache_file.getAbsolutePath(), 0);
                                                } else if (final_image_orientation.equals("90")) {
                                                    imageSelected(cache_file.getAbsolutePath(), 90);
                                                } else if (final_image_orientation.equals("180")) {
                                                    imageSelected(cache_file.getAbsolutePath(), 180);
                                                } else if (final_image_orientation.equals("270")) {
                                                    imageSelected(cache_file.getAbsolutePath(), 270);
                                                } else {
                                                    imageSelected(cache_file.getAbsolutePath(), 0);
                                                }
                                            }
                                        });
                                    } else {
                                        throw(new Exception("CACHE_DIR_ACCESS_FAILED"));
                                    }
                                } else {
                                    throw(new Exception("BITMAP_DECODE_FAILED"));
                                }
                            } catch (Exception ex) {
                                runOnUiThread(new Runnable() {
                                    @Override
                                    public void run()
                                    {
                                        imageSelectionFailed();
                                    }
                                });
                            } finally {
                                if (input_stream != null) {
                                    try {
                                        input_stream.close();
                                    } catch (Exception ex) {
                                        // Ignore
                                    }
                                }
                                if (output_stream != null) {
                                    try {
                                        output_stream.close();
                                    } catch (Exception ex) {
                                        // Ignore
                                    }
                                }
                                if (file_output_stream != null) {
                                    try {
                                        file_output_stream.close();
                                    } catch (Exception ex) {
                                        // Ignore
                                    }
                                }
                            }
                        }
                    })).start();
                } else {
                    imageSelectionFailed();
                }
            } else {
                imageSelectionCancelled();
            }
        }
    }
}
