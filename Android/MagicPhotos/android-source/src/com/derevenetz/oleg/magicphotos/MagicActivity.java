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
                                 REQUEST_CODE_SHOW_GALLERY            = 1001,
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
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);

            intent.setType("image/*");

            instance.startActivityForResult(intent, REQUEST_CODE_SHOW_GALLERY);
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

        if (requestCode == REQUEST_CODE_SHOW_GALLERY) {
            if (resultCode == RESULT_OK && data != null) {
                Uri image_uri = data.getData();

                if (image_uri != null) {
                    String[] query_columns = { MediaStore.Images.Media.DATA, MediaStore.Images.Media.ORIENTATION };

                    Cursor cursor = getContentResolver().query(image_uri, query_columns, null, null, null);

                    if (cursor != null) {
                        if (cursor.moveToFirst()) {
                            String image_file        = cursor.getString(cursor.getColumnIndex(query_columns[0]));
                            String image_orientation = cursor.getString(cursor.getColumnIndex(query_columns[1]));

                            if (image_orientation.equals("0")) {
                                imageSelected(image_file, 0);
                            } else if (image_orientation.equals("90")) {
                                imageSelected(image_file, 90);
                            } else if (image_orientation.equals("180")) {
                                imageSelected(image_file, 180);
                            } else if (image_orientation.equals("270")) {
                                imageSelected(image_file, 270);
                            } else {
                                imageSelected(image_file, 0);
                            }
                        } else {
                            imageSelectionCancelled();
                        }

                        cursor.close();
                    } else {
                        imageSelectionCancelled();
                    }
                } else {
                    imageSelectionCancelled();
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
