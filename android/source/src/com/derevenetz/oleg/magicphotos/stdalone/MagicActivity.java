package com.derevenetz.oleg.magicphotos.stdalone;

import java.io.File;
import java.io.InputStream;
import java.io.FileOutputStream;

import android.content.Context;
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
import android.support.v4.content.FileProvider;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import org.qtproject.qt5.android.bindings.QtActivity;

import com.google.ads.mediation.admob.AdMobAdapter;

import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.MobileAds;

public class MagicActivity extends QtActivity
{
    private static final int  REQUEST_CODE_SHOW_GALLERY  = 1001;

    private static final long AD_RELOAD_ON_FAILURE_DELAY = 60000;

    private boolean           statusBarVisible           = false,
                              showPersonalizedAds        = false;
    private int               statusBarHeight            = 0;
    private AdView            bannerView                 = null;
    private InterstitialAd    interstitial               = null;

    private static native void bannerViewHeightUpdated(int height);

    private static native void imageSelected(String image_file, int image_orientation);
    private static native void imageSelectionCancelled();
    private static native void imageSelectionFailed();

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

                    if (bannerView != null) {
                        int ad_visibility = bannerView.getVisibility();

                        bannerView.setVisibility(View.GONE);
                        bannerView.setY(statusBarHeight);
                        bannerView.setVisibility(ad_visibility);
                    }
                } else {
                    statusBarVisible = false;

                    if (bannerView != null) {
                        int ad_visibility = bannerView.getVisibility();

                        bannerView.setVisibility(View.GONE);
                        bannerView.setY(0);
                        bannerView.setVisibility(ad_visibility);
                    }
                }
            }
        });
    }

    @Override
    public void onResume()
    {
        super.onResume();

        if (bannerView != null) {
            bannerView.resume();
        }
    }

    @Override
    public void onPause()
    {
        if (bannerView != null) {
            bannerView.pause();
        }

        super.onPause();
    }

    @Override
    public void onDestroy()
    {
        if (bannerView != null) {
            bannerView.destroy();

            bannerView = null;
        }

        super.onDestroy();
    }

    public int getScreenDPI()
    {
        DisplayMetrics metrics = getResources().getDisplayMetrics();

        return metrics.densityDpi;
    }

    public String getSaveDirectory()
    {
        File save_dir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).getAbsolutePath() + "/MagicPhotos");

        if (save_dir.mkdirs() || save_dir.isDirectory()) {
            return save_dir.getAbsolutePath();
        } else {
            return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).getAbsolutePath();
        }
    }

    public void showGallery()
    {
        try {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);

            intent.setType("image/*");

            startActivityForResult(intent, REQUEST_CODE_SHOW_GALLERY);
        } catch (Exception ex) {
            imageSelectionFailed();
        }
    }

    public void refreshGallery(String image_file)
    {
        MediaScannerConnection.scanFile(this, new String[] { image_file }, null, null);
    }

    public void shareImage(String image_file)
    {
        try {
            Intent intent = new Intent(Intent.ACTION_SEND);

            intent.setType("image/*");
            intent.putExtra(Intent.EXTRA_STREAM, FileProvider.getUriForFile(this, BuildConfig.APPLICATION_ID + ".provider", new File(image_file)));

            startActivity(Intent.createChooser(intent, getResources().getString(R.string.share_image_chooser_title)));
        } catch (Exception ex) {
            // Ignore
        }
    }

    public void initAds(String app_id, String interstitial_unit_id)
    {
        final String  f_app_id               = app_id;
        final String  f_interstitial_unit_id = interstitial_unit_id;
        final Context f_context              = this;

        runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                MobileAds.initialize(f_context, f_app_id);

                interstitial = new InterstitialAd(f_context);

                interstitial.setAdUnitId(f_interstitial_unit_id);

                interstitial.setAdListener(new AdListener() {
                    @Override
                    public void onAdClosed()
                    {
                        if (interstitial != null) {
                            AdRequest.Builder builder = new AdRequest.Builder();

                            if (showPersonalizedAds) {
                                interstitial.loadAd(builder.setMaxAdContentRating(AdRequest.MAX_AD_CONTENT_RATING_G)
                                                           .tagForChildDirectedTreatment(true)
                                                           .build());
                            } else {
                                Bundle extras = new Bundle();

                                extras.putString("npa", "1");

                                interstitial.loadAd(builder.setMaxAdContentRating(AdRequest.MAX_AD_CONTENT_RATING_G)
                                                           .tagForChildDirectedTreatment(true)
                                                           .addNetworkExtrasBundle(AdMobAdapter.class, extras)
                                                           .build());
                            }
                        }
                    }

                    @Override
                    public void onAdFailedToLoad(int errorCode)
                    {
                        if (interstitial != null) {
                            new Handler().postDelayed(new Runnable() {
                                @Override
                                public void run()
                                {
                                    if (interstitial != null) {
                                        AdRequest.Builder builder = new AdRequest.Builder();

                                        if (showPersonalizedAds) {
                                            interstitial.loadAd(builder.setMaxAdContentRating(AdRequest.MAX_AD_CONTENT_RATING_G)
                                                                       .tagForChildDirectedTreatment(true)
                                                                       .build());
                                        } else {
                                            Bundle extras = new Bundle();

                                            extras.putString("npa", "1");

                                            interstitial.loadAd(builder.setMaxAdContentRating(AdRequest.MAX_AD_CONTENT_RATING_G)
                                                                       .tagForChildDirectedTreatment(true)
                                                                       .addNetworkExtrasBundle(AdMobAdapter.class, extras)
                                                                       .build());
                                        }
                                    }
                                }
                            }, AD_RELOAD_ON_FAILURE_DELAY);
                        }
                    }
                });

                AdRequest.Builder builder = new AdRequest.Builder();

                if (showPersonalizedAds) {
                    interstitial.loadAd(builder.setMaxAdContentRating(AdRequest.MAX_AD_CONTENT_RATING_G)
                                               .tagForChildDirectedTreatment(true)
                                               .build());
                } else {
                    Bundle extras = new Bundle();

                    extras.putString("npa", "1");

                    interstitial.loadAd(builder.setMaxAdContentRating(AdRequest.MAX_AD_CONTENT_RATING_G)
                                               .tagForChildDirectedTreatment(true)
                                               .addNetworkExtrasBundle(AdMobAdapter.class, extras)
                                               .build());
                }
            }
        });
    }

    public void setAdsPersonalization(boolean personalized)
    {
        final boolean f_personalized = personalized;

        runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                showPersonalizedAds = f_personalized;
            }
        });
    }

    public void showBannerView(String unit_id)
    {
        final String  f_unit_id = unit_id;
        final Context f_context = this;

        runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                View view = getWindow().getDecorView().getRootView();

                if (view instanceof ViewGroup) {
                    ViewGroup view_group = (ViewGroup)view;

                    if (bannerView != null) {
                        view_group.removeView(bannerView);

                        bannerView.destroy();

                        bannerViewHeightUpdated(0);

                        bannerView = null;
                    }

                    FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT,
                                                                                   FrameLayout.LayoutParams.WRAP_CONTENT,
                                                                                   Gravity.CENTER_HORIZONTAL);

                    bannerView = new AdView(f_context);

                    bannerView.setAdSize(AdSize.SMART_BANNER);
                    bannerView.setAdUnitId(f_unit_id);
                    bannerView.setLayoutParams(params);
                    bannerView.setVisibility(View.GONE);

                    if (statusBarVisible) {
                        bannerView.setY(statusBarHeight);
                    } else {
                        bannerView.setY(0);
                    }

                    bannerView.setAdListener(new AdListener() {
                        @Override
                        public void onAdLoaded()
                        {
                            if (bannerView != null) {
                                bannerView.setVisibility(View.VISIBLE);

                                bannerView.post(new Runnable() {
                                    @Override
                                    public void run()
                                    {
                                        if (bannerView != null) {
                                            bannerViewHeightUpdated(bannerView.getHeight());
                                        }
                                    }
                                });
                            }
                        }

                        @Override
                        public void onAdFailedToLoad(int errorCode)
                        {
                            if (bannerView != null) {
                                bannerView.setVisibility(View.VISIBLE);

                                bannerView.post(new Runnable() {
                                    @Override
                                    public void run()
                                    {
                                        if (bannerView != null) {
                                            bannerViewHeightUpdated(bannerView.getHeight());
                                        }
                                    }
                                });
                            }
                        }
                    });

                    view_group.addView(bannerView);

                    AdRequest.Builder builder = new AdRequest.Builder();

                    if (showPersonalizedAds) {
                        bannerView.loadAd(builder.setMaxAdContentRating(AdRequest.MAX_AD_CONTENT_RATING_G)
                                                 .tagForChildDirectedTreatment(true)
                                                 .build());
                    } else {
                        Bundle extras = new Bundle();

                        extras.putString("npa", "1");

                        bannerView.loadAd(builder.setMaxAdContentRating(AdRequest.MAX_AD_CONTENT_RATING_G)
                                                 .tagForChildDirectedTreatment(true)
                                                 .addNetworkExtrasBundle(AdMobAdapter.class, extras)
                                                 .build());
                    }
                }
            }
        });
    }

    public void hideBannerView()
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                View view = getWindow().getDecorView().getRootView();

                if (view instanceof ViewGroup) {
                    ViewGroup view_group = (ViewGroup)view;

                    if (bannerView != null) {
                        view_group.removeView(bannerView);

                        bannerView.destroy();

                        bannerViewHeightUpdated(0);

                        bannerView = null;
                    }
                }
            }
        });
    }

    public void showInterstitial()
    {
        runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                if (interstitial != null && interstitial.isLoaded()) {
                    interstitial.show();
                }
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == REQUEST_CODE_SHOW_GALLERY) {
            if (resultCode == RESULT_OK && data != null) {
                final Uri image_uri = data.getData();

                if (image_uri != null) {
                    (new Thread(new Runnable() {
                        @Override
                        public void run()
                        {
                            try (InputStream input_stream = getContentResolver().openInputStream(image_uri)) {
                                Bitmap bitmap = BitmapFactory.decodeStream(input_stream);

                                if (bitmap != null) {
                                    final File cache_file = new File(getApplicationContext().getCacheDir().getAbsolutePath() + "/cache.jpg");

                                    if (cache_file.getParentFile().mkdirs() || cache_file.getParentFile().isDirectory()) {
                                        try (FileOutputStream output_stream = new FileOutputStream(cache_file)) {
                                            bitmap.compress(Bitmap.CompressFormat.JPEG, 90, output_stream);
                                        }

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

                                        final String f_image_orientation = image_orientation;

                                        runOnUiThread(new Runnable() {
                                            @Override
                                            public void run()
                                            {
                                                if (f_image_orientation == null) {
                                                    imageSelected(cache_file.getAbsolutePath(), 0);
                                                } else if (f_image_orientation.equals("0")) {
                                                    imageSelected(cache_file.getAbsolutePath(), 0);
                                                } else if (f_image_orientation.equals("90")) {
                                                    imageSelected(cache_file.getAbsolutePath(), 90);
                                                } else if (f_image_orientation.equals("180")) {
                                                    imageSelected(cache_file.getAbsolutePath(), 180);
                                                } else if (f_image_orientation.equals("270")) {
                                                    imageSelected(cache_file.getAbsolutePath(), 270);
                                                } else {
                                                    imageSelected(cache_file.getAbsolutePath(), 0);
                                                }
                                            }
                                        });
                                    } else {
                                        throw(new Exception("Cannot access cache directory"));
                                    }
                                } else {
                                    throw(new Exception("Bitmap decode failed"));
                                }
                            } catch (Exception ex) {
                                runOnUiThread(new Runnable() {
                                    @Override
                                    public void run()
                                    {
                                        imageSelectionFailed();
                                    }
                                });
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
