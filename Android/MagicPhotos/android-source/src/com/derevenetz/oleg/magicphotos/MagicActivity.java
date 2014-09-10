package com.derevenetz.oleg.magicphotos;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.media.ExifInterface;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.Bundle;
import android.provider.MediaStore;
import android.util.DisplayMetrics;

import org.qtproject.qt5.android.bindings.QtApplication;
import org.qtproject.qt5.android.bindings.QtActivity;

public class MagicActivity extends QtActivity
{
    private static int RESULT_LOAD_IMAGE = 1;

    private static MagicActivity instance = null;

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
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
    }

    public static int getScreenDPI()
    {
        DisplayMetrics metrics = instance.getResources().getDisplayMetrics();

        return metrics.densityDpi;
    }

    public static void showGallery()
    {
        instance.startActivityForResult(new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI), RESULT_LOAD_IMAGE);
    }

    public static void refreshGallery(String image_file)
    {
        MediaScannerConnection.scanFile(instance, new String[] { image_file }, null, null);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
       super.onActivityResult(requestCode, resultCode, data);

       if (requestCode == RESULT_LOAD_IMAGE) {
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
       }
   }
}
