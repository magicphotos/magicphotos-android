package com.derevenetz.oleg.magicphotos;

import java.io.File;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.util.ArrayList;

import org.json.JSONObject;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.util.DisplayMetrics;

import com.android.vending.billing.*;

import org.qtproject.qt5.android.bindings.QtApplication;
import org.qtproject.qt5.android.bindings.QtActivity;

public class MagicActivity extends QtActivity
{
    private static final boolean DEBUG_GOOGLE_IAP_AUTO_CONSUME        = false,
                                 DEBUG_GOOGLE_IAP_ALWAYS_TRIAL        = false,
                                 DEBUG_GOOGLE_IAP_ALWAYS_FULL         = false,
                                 PROMO_FULL_VERSION                   = true;

    private static final int     GOOGLE_IAP_RESULT_OK                 = 0,
                                 GOOGLE_IAP_RESULT_ITEM_ALREADY_OWNED = 7,
                                 REQUEST_CODE_SHOW_GALLERY            = 1001,
                                 REQUEST_CODE_BUY_FULL_VERSION        = 1002;

    private static final String  GOOGLE_IAP_FULL_VERSION_PRODUCT_ID   = "magicphotos.version.full",
                                 GOOGLE_IAP_DEVELOPER_PAYLOAD         = "PXV0HzqSbr1ZT";

    private static boolean       isFullVersion                        = false,
                                 isPromoFullVersion                   = false;
    private static MagicActivity instance                             = null;

    private IInAppBillingService androidIAPService                    = null;

    private ServiceConnection androidIAPServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service)
        {
            androidIAPService = IInAppBillingService.Stub.asInterface(service);

            (new Thread(new Runnable() {
                @Override
                public void run() {
                    try {
                        Bundle owned_items = androidIAPService.getPurchases(3, getPackageName(), "inapp", null);

                        if (owned_items.getInt("RESPONSE_CODE", -1) == GOOGLE_IAP_RESULT_OK) {
                            boolean           is_full_version   = false;
                            ArrayList<String> owned_products    = owned_items.getStringArrayList("INAPP_PURCHASE_ITEM_LIST");
                            ArrayList<String> product_purchases = owned_items.getStringArrayList("INAPP_PURCHASE_DATA_LIST");

                            for (int i = 0; i < owned_products.size(); i++) {
                                String product  = owned_products.get(i);
                                String purchase = product_purchases.get(i);

                                if (product.equals(GOOGLE_IAP_FULL_VERSION_PRODUCT_ID)) {
                                    is_full_version = true;

                                    if (DEBUG_GOOGLE_IAP_AUTO_CONSUME) {
                                        androidIAPService.consumePurchase(3, getPackageName(), (new JSONObject(purchase)).getString("purchaseToken"));
                                    }
                                }
                            }

                            if (DEBUG_GOOGLE_IAP_ALWAYS_TRIAL) {
                                isFullVersion = false;
                            } else if (DEBUG_GOOGLE_IAP_ALWAYS_FULL) {
                                isFullVersion = true;
                            } else if (isPromoFullVersion) {
                                isFullVersion = true;
                            } else {
                                isFullVersion = is_full_version;

                                SharedPreferences.Editor editor = getPreferences(MODE_PRIVATE).edit();

                                editor.putBoolean("FullVersion", isFullVersion);
                                editor.commit();
                            }
                        }
                    } catch (Exception ex) {
                        // Ignored
                    }
                }
            })).start();
        }

        @Override
        public void onServiceDisconnected(ComponentName name)
        {
            androidIAPService = null;
        }
    };

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

        int prev_version_code = getPreferences(MODE_PRIVATE).getInt("VersionCode", 0);

        try {
            SharedPreferences.Editor editor = getPreferences(MODE_PRIVATE).edit();

            editor.putInt("VersionCode", getPackageManager().getPackageInfo(getPackageName(), 0).versionCode);
            editor.commit();
        } catch (Exception ex) {
            // Ignored
        }

        if (DEBUG_GOOGLE_IAP_ALWAYS_TRIAL) {
            isFullVersion = false;
        } else if (DEBUG_GOOGLE_IAP_ALWAYS_FULL) {
            isFullVersion = true;
        } else if (PROMO_FULL_VERSION && prev_version_code == 0) {
            isFullVersion      = true;
            isPromoFullVersion = true;

            SharedPreferences.Editor editor = getPreferences(MODE_PRIVATE).edit();

            editor.putBoolean("PromoFullVersion", true);
            editor.commit();
        } else if (getPreferences(MODE_PRIVATE).getBoolean("PromoFullVersion", false)) {
            isFullVersion      = true;
            isPromoFullVersion = true;
        } else {
            isFullVersion = getPreferences(MODE_PRIVATE).getBoolean("FullVersion", false);
        }

        Intent intent = new Intent("com.android.vending.billing.InAppBillingService.BIND");

        intent.setPackage("com.android.vending");

        bindService(intent, androidIAPServiceConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();

        if (androidIAPService != null) {
            unbindService(androidIAPServiceConnection);
        }
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

    public static boolean getFullVersion()
    {
        return isFullVersion;
    }

    public static boolean buyFullVersion()
    {
        try {
            if (instance.androidIAPService != null) {
                Bundle intent_bundle = instance.androidIAPService.getBuyIntent(3, instance.getPackageName(), GOOGLE_IAP_FULL_VERSION_PRODUCT_ID, "inapp", GOOGLE_IAP_DEVELOPER_PAYLOAD);

                if (intent_bundle.getInt("RESPONSE_CODE", -1) == GOOGLE_IAP_RESULT_OK) {
                    PendingIntent pending_intent = intent_bundle.getParcelable("BUY_INTENT");

                    instance.startIntentSenderForResult(pending_intent.getIntentSender(), REQUEST_CODE_BUY_FULL_VERSION, new Intent(), Integer.valueOf(0), Integer.valueOf(0), Integer.valueOf(0));

                    return true;
                } else if (intent_bundle.getInt("RESPONSE_CODE", -1) == GOOGLE_IAP_RESULT_ITEM_ALREADY_OWNED) {
                    isFullVersion = true;

                    SharedPreferences.Editor editor = instance.getPreferences(MODE_PRIVATE).edit();

                    editor.putBoolean("FullVersion", isFullVersion);
                    editor.commit();

                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } catch (Exception ex) {
            return false;
        }
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

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == REQUEST_CODE_SHOW_GALLERY) {
            if (resultCode == RESULT_OK && data != null) {
                final Uri image_uri = data.getData();

                if (image_uri != null) {
                    (new Thread(new Runnable() {
                        @Override
                        public void run() {
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
                                            public void run() {
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
                                    public void run() {
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
        } else if (requestCode == REQUEST_CODE_BUY_FULL_VERSION) {
            if (data.getIntExtra("RESPONSE_CODE", -1) == GOOGLE_IAP_RESULT_OK) {
                try {
                    JSONObject object = new JSONObject(data.getStringExtra("INAPP_PURCHASE_DATA"));

                    if (object.getString("productId").equals(GOOGLE_IAP_FULL_VERSION_PRODUCT_ID) &&
                        object.getString("developerPayload").equals(GOOGLE_IAP_DEVELOPER_PAYLOAD)) {
                        isFullVersion = true;

                        SharedPreferences.Editor editor = getPreferences(MODE_PRIVATE).edit();

                        editor.putBoolean("FullVersion", isFullVersion);
                        editor.commit();
                    }
                } catch (Exception ex) {
                    // Ignored
                }
            }
        }
    }
}
