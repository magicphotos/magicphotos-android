package com.derevenetz.oleg.magicphotos.stdalone;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.webkit.MimeTypeMap;
import android.widget.FrameLayout;

import androidx.core.content.FileProvider;
import androidx.exifinterface.media.ExifInterface;

import org.qtproject.qt5.android.bindings.QtActivity;

import org.apache.commons.io.IOUtils;

import com.google.ads.mediation.admob.AdMobAdapter;

import com.google.android.gms.ads.AdListener;
import com.google.android.gms.ads.AdRequest;
import com.google.android.gms.ads.AdSize;
import com.google.android.gms.ads.AdView;
import com.google.android.gms.ads.InterstitialAd;
import com.google.android.gms.ads.MobileAds;
import com.google.android.gms.ads.RequestConfiguration;

public class MagicActivity extends QtActivity
{
    private static final int  REQUEST_CODE_SHOW_GALLERY  = 1001;

    private static final long AD_RELOAD_ON_FAILURE_DELAY = 60000;

    private boolean           statusBarVisible           = false,
                              showPersonalizedAds        = false;
    private int               statusBarHeight            = 0;
    private AdView            bannerView                 = null;
    private InterstitialAd    interstitial               = null;

    private static native void deviceConfigurationUpdated();

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

    @Override
    public void onConfigurationChanged(Configuration newConfig)
    {
        super.onConfigurationChanged(newConfig);

        deviceConfigurationUpdated();
    }

    public int getScreenDpi()
    {
        DisplayMetrics metrics = getResources().getDisplayMetrics();

        return metrics.densityDpi;
    }

    public boolean getNightModeStatus()
    {
        Configuration config = getResources().getConfiguration();

        return ((config.uiMode & Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES);
    }

    public String getSaveDirectory()
    {
        File save_dir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).getAbsolutePath(), "MagicPhotos");

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
        MediaScannerConnection.scanFile(this, new String[]{image_file}, null, null);
    }

    public void shareImage(String image_file)
    {
        try {
            Intent intent = new Intent(Intent.ACTION_SEND);

            intent.setType("image/*");
            intent.putExtra(Intent.EXTRA_STREAM, FileProvider.getUriForFile(this, BuildConfig.APPLICATION_ID + ".provider", new File(image_file)));

            startActivity(Intent.createChooser(intent, getResources().getString(R.string.share_image_chooser_title)));
        } catch (Exception ex) {
            Log.e("MagicActivity", "shareImage() : " + ex.toString());
        }
    }

    public void initAds(String interstitial_unit_id)
    {
        final String  f_interstitial_unit_id = interstitial_unit_id;
        final Context f_context              = this;

        runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                MobileAds.setRequestConfiguration(MobileAds.getRequestConfiguration()
                                                           .toBuilder().setMaxAdContentRating(RequestConfiguration.MAX_AD_CONTENT_RATING_G)
                                                                       .build());

                MobileAds.initialize(f_context);

                interstitial = new InterstitialAd(f_context);

                interstitial.setAdUnitId(f_interstitial_unit_id);

                interstitial.setAdListener(new AdListener() {
                    @Override
                    public void onAdClosed()
                    {
                        if (interstitial != null) {
                            AdRequest.Builder builder = new AdRequest.Builder();

                            if (showPersonalizedAds) {
                                interstitial.loadAd(builder.build());
                            } else {
                                Bundle extras = new Bundle();

                                extras.putString("npa", "1");

                                interstitial.loadAd(builder.addNetworkExtrasBundle(AdMobAdapter.class, extras)
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
                                            interstitial.loadAd(builder.build());
                                        } else {
                                            Bundle extras = new Bundle();

                                            extras.putString("npa", "1");

                                            interstitial.loadAd(builder.addNetworkExtrasBundle(AdMobAdapter.class, extras)
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
                    interstitial.loadAd(builder.build());
                } else {
                    Bundle extras = new Bundle();

                    extras.putString("npa", "1");

                    interstitial.loadAd(builder.addNetworkExtrasBundle(AdMobAdapter.class, extras)
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
                                                                                   Gravity.TOP | Gravity.CENTER_HORIZONTAL);

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
                            }
                        }

                        @Override
                        public void onAdFailedToLoad(int errorCode)
                        {
                            if (bannerView != null) {
                                bannerView.setVisibility(View.VISIBLE);
                            }
                        }
                    });

                    bannerView.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
                        @Override
                        public void onGlobalLayout()
                        {
                            if (bannerView != null) {
                                int height = bannerView.getHeight();

                                if (height > 0) {
                                    bannerViewHeightUpdated(height);
                                } else {
                                    bannerViewHeightUpdated(0);
                                }
                            }
                        }
                    });

                    view_group.addView(bannerView);

                    AdRequest.Builder builder = new AdRequest.Builder();

                    if (showPersonalizedAds) {
                        bannerView.loadAd(builder.build());
                    } else {
                        Bundle extras = new Bundle();

                        extras.putString("npa", "1");

                        bannerView.loadAd(builder.addNetworkExtrasBundle(AdMobAdapter.class, extras)
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
                                String image_content_type = getContentResolver().getType(image_uri);

                                if (image_content_type != null) {
                                    String cache_file_extension = MimeTypeMap.getSingleton().getExtensionFromMimeType(image_content_type);

                                    if (cache_file_extension != null) {
                                        final File cache_file = new File(getApplicationContext().getCacheDir().getAbsolutePath(), "cache." + cache_file_extension);

                                        try (FileOutputStream output_stream = new FileOutputStream(cache_file)) {
                                            IOUtils.copy(input_stream, output_stream);
                                        }

                                        final int image_orientation = (new ExifInterface(cache_file.getAbsolutePath())).getAttributeInt(ExifInterface.TAG_ORIENTATION,
                                                                                                                                        ExifInterface.ORIENTATION_NORMAL);

                                        runOnUiThread(new Runnable() {
                                            @Override
                                            public void run()
                                            {
                                                if (image_orientation == ExifInterface.ORIENTATION_ROTATE_90) {
                                                    imageSelected(cache_file.getAbsolutePath(), 90);
                                                } else if (image_orientation == ExifInterface.ORIENTATION_ROTATE_180) {
                                                    imageSelected(cache_file.getAbsolutePath(), 180);
                                                } else if (image_orientation == ExifInterface.ORIENTATION_ROTATE_270) {
                                                    imageSelected(cache_file.getAbsolutePath(), 270);
                                                } else {
                                                    imageSelected(cache_file.getAbsolutePath(), 0);
                                                }
                                            }
                                        });
                                    } else {
                                        throw(new Exception("Unknown image content type"));
                                    }
                                } else {
                                    throw(new Exception("Invalid image content type"));
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
