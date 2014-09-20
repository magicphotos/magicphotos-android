package com.derevenetz.oleg.magicphotos;

import java.io.File;
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
import android.media.ExifInterface;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.provider.MediaStore;
import android.util.DisplayMetrics;

import com.android.vending.billing.*;

import org.qtproject.qt5.android.bindings.QtApplication;
import org.qtproject.qt5.android.bindings.QtActivity;

public class MagicActivity extends QtActivity
{
    private static final boolean DEBUG_GOOGLE_IAP_AUTO_CONSUME        = false,
                                 DEBUG_GOOGLE_IAP_ALWAYS_TRIAL        = false,
                                 DEBUG_GOOGLE_IAP_ALWAYS_FULL         = false;

    private static final int     GOOGLE_IAP_RESULT_OK                 = 0,
                                 GOOGLE_IAP_RESULT_ITEM_ALREADY_OWNED = 7,
                                 REQUEST_CODE_LOAD_IMAGE              = 1001,
                                 REQUEST_CODE_BUY_FULL_VERSION        = 1002;

    private static final String  GOOGLE_IAP_FULL_VERSION_PRODUCT_ID   = "magicphotos.version.full",
                                 GOOGLE_IAP_DEVELOPER_PAYLOAD         = "PXV0HzqSbr1ZT";

    private static boolean       isFullVersion                        = false;
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
                            } else {
                                isFullVersion = is_full_version;
                            }

                            SharedPreferences.Editor editor = getPreferences(MODE_PRIVATE).edit();

                            editor.putBoolean("FullVersion", isFullVersion);
                            editor.commit();
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

    public MagicActivity()
    {
        instance = this;
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        if (DEBUG_GOOGLE_IAP_ALWAYS_TRIAL) {
            isFullVersion = false;
        } else if (DEBUG_GOOGLE_IAP_ALWAYS_FULL) {
            isFullVersion = true;
        } else {
            isFullVersion = getPreferences(MODE_PRIVATE).getBoolean("FullVersion", false);
        }

        bindService(new Intent("com.android.vending.billing.InAppBillingService.BIND"), androidIAPServiceConnection, Context.BIND_AUTO_CREATE);
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
            instance.startActivityForResult(new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI), REQUEST_CODE_LOAD_IMAGE);
        } catch (Exception ex) {
            imageSelectionCancelled();
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

        if (requestCode == REQUEST_CODE_LOAD_IMAGE) {
            if (resultCode == RESULT_OK && data != null) {
                Uri      selected_image   = data.getData();
                String[] file_path_column = { MediaStore.Images.Media.DATA };

                Cursor cursor = getContentResolver().query(selected_image, file_path_column, null, null, null);

                cursor.moveToFirst();

                int    index      = cursor.getColumnIndex(file_path_column[0]);
                String image_file = cursor.getString(index);

                cursor.close();

                int image_orientation = ExifInterface.ORIENTATION_UNDEFINED;

                try {
                    ExifInterface exif_interface = new ExifInterface(image_file);

                    image_orientation = exif_interface.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_UNDEFINED);

                    if (image_orientation < 0) {
                        image_orientation = ExifInterface.ORIENTATION_UNDEFINED;
                    }
                } catch (Exception ex) {
                    image_orientation = ExifInterface.ORIENTATION_UNDEFINED;
                }

                imageSelected(image_file, image_orientation);
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
