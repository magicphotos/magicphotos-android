using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Net;
using System.IO;
using System.IO.IsolatedStorage;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using Microsoft.Phone;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Info;
using Microsoft.Phone.Tasks;
using Microsoft.Phone.Shell;
using Microsoft.Xna.Framework.Media;

namespace MagicPhotos
{
    public partial class BlurPreviewPage : PhoneApplicationPage
    {
        private const double REDUCTION_MPIX_LIMIT = 1.0,
                             PREVIEW_MPIX_LIMIT   = 0.5;

        class BlurGenTaskData
        {
            public int   radius;
            public int   width, height;
            public int[] pixels;
        }

        private bool             loadImageOnLayoutUpdate,
                                 loadImageCancelled,
                                 pageNavigationComplete,
                                 needImageReduction,
                                 restartBlurGenerator;
        private int              gaussianRadius;
        private WriteableBitmap  loadedBitmap,
                                 scaledBitmap,
                                 previewBitmap;
        private BackgroundWorker blurGeneratorWorker;
        private PhotoChooserTask photoChooserTask;

        public BlurPreviewPage()
        {
            InitializeComponent();

            this.loadImageOnLayoutUpdate = true;
            this.loadImageCancelled      = false;
            this.pageNavigationComplete  = false;
            this.restartBlurGenerator    = false;
            this.gaussianRadius          = (int)this.GaussianRadiusSlider.Value;
            this.loadedBitmap            = null;
            this.scaledBitmap            = null;
            this.previewBitmap           = null;

            try
            {
                long limit = (long)DeviceExtendedProperties.GetValue("ApplicationWorkingSetLimit");

                if (limit <= 90L * 1024L * 1024L)
                {
                    this.needImageReduction = true;
                }
                else
                {
                    this.needImageReduction = false;
                }
            }
            catch (Exception)
            {
                this.needImageReduction = false;
            }

            this.blurGeneratorWorker                     = new BackgroundWorker();
            this.blurGeneratorWorker.DoWork             += new DoWorkEventHandler(blurGeneratorWorker_DoWork);
            this.blurGeneratorWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(blurGeneratorWorker_RunWorkerCompleted);

            this.photoChooserTask            = new PhotoChooserTask();
            this.photoChooserTask.ShowCamera = true;
            this.photoChooserTask.Completed += new EventHandler<PhotoResult>(photoChooserTask_Completed);

            ApplicationBarIconButton button;

            button        = new ApplicationBarIconButton(new Uri("/Images/help.png", UriKind.Relative));
            button.Text   = AppResources.ApplicationBarButtonHelpText;
            button.Click += new EventHandler(HelpButton_Click);

            this.ApplicationBar.Buttons.Add(button);
        }

        protected override void OnNavigatedTo(System.Windows.Navigation.NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            this.pageNavigationComplete = true;

            if (this.loadImageCancelled)
            {
                if (NavigationService.CanGoBack)
                {
                    NavigationService.GoBack();
                }

                this.loadImageCancelled = false;
            }
        }

        private void StartBlurGenerator()
        {
            if (this.scaledBitmap != null)
            {
                try
                {
                    BlurGenTaskData task_data = new BlurGenTaskData();

                    task_data.radius = this.gaussianRadius;
                    task_data.width  = this.scaledBitmap.PixelWidth;
                    task_data.height = this.scaledBitmap.PixelHeight;
                    task_data.pixels = this.scaledBitmap.Pixels;

                    this.blurGeneratorWorker.RunWorkerAsync(task_data);

                    this.GenerationProgressIndicator.IsVisible = true;
                }
                catch (Exception)
                {
                    // Ignore
                }
            }
        }

        private void LoadImage(WriteableBitmap bitmap)
        {
            if (this.needImageReduction && bitmap.PixelWidth * bitmap.PixelHeight > REDUCTION_MPIX_LIMIT * 1000000.0)
            {
                double factor = Math.Sqrt((bitmap.PixelWidth * bitmap.PixelHeight) / (REDUCTION_MPIX_LIMIT * 1000000.0));

                bitmap = bitmap.Resize((int)(bitmap.PixelWidth / factor), (int)(bitmap.PixelHeight / factor), WriteableBitmapExtensions.Interpolation.NearestNeighbor);
            }

            if (bitmap.PixelWidth != 0 && bitmap.PixelHeight != 0)
            {
                this.loadedBitmap = bitmap;

                if (bitmap.PixelWidth * bitmap.PixelHeight > PREVIEW_MPIX_LIMIT * 1000000.0)
                {
                    double factor = Math.Sqrt((bitmap.PixelWidth * bitmap.PixelHeight) / (PREVIEW_MPIX_LIMIT * 1000000.0));

                    bitmap = bitmap.Resize((int)(bitmap.PixelWidth / factor), (int)(bitmap.PixelHeight / factor), WriteableBitmapExtensions.Interpolation.NearestNeighbor);
                }

                if (bitmap.PixelWidth != 0 && bitmap.PixelHeight != 0)
                {
                    this.scaledBitmap = bitmap;

                    StartBlurGenerator();
                }
            }
        }

        private void BlurPreviewPage_LayoutUpdated(object sender, EventArgs e)
        {
            if (this.loadImageOnLayoutUpdate && this.pageNavigationComplete)
            {
                try
                {
                    using (IsolatedStorageFile store = IsolatedStorageFile.GetUserStoreForApplication())
                    {
                        string file_name = "image.jpg";

                        if (store.FileExists(file_name))
                        {
                            using (IsolatedStorageFileStream stream = store.OpenFile(file_name, FileMode.Open, FileAccess.Read))
                            {
                                WriteableBitmap bitmap = PictureDecoder.DecodeJpeg(stream);

                                LoadImage(bitmap);
                            }

                            store.DeleteFile(file_name);
                        }
                        else
                        {
                            this.photoChooserTask.Show();
                        }
                    }
                }
                catch (Exception ex)
                {
                    ThreadPool.QueueUserWorkItem((stateInfo) =>
                    {
                        Deployment.Current.Dispatcher.BeginInvoke(delegate()
                        {
                            try
                            {
                                MessageBox.Show(AppResources.MessageBoxMessageImageOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                            }
                            catch (Exception)
                            {
                                // Ignore
                            }
                        });
                    });
                }

                this.loadImageOnLayoutUpdate = false;
            }
        }

        private void ApplyButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.loadedBitmap != null)
            {
                try
                {
                    using (IsolatedStorageFile store = IsolatedStorageFile.GetUserStoreForApplication())
                    {
                        string file_name = "image.jpg";

                        if (store.FileExists(file_name))
                        {
                            store.DeleteFile(file_name);
                        }

                        using (IsolatedStorageFileStream stream = store.CreateFile(file_name))
                        {
                            this.loadedBitmap.SaveJpeg(stream, this.loadedBitmap.PixelWidth, this.loadedBitmap.PixelHeight, 0, 100);
                        }

                        NavigationService.Navigate(new Uri(string.Format("/BlurPage.xaml?radius={0}", Uri.EscapeUriString(this.gaussianRadius.ToString())), UriKind.Relative));
                    }
                }
                catch (Exception ex)
                {
                    try
                    {
                        MessageBox.Show(AppResources.MessageBoxMessageImageOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                    }
                    catch (Exception)
                    {
                        // Ignore
                    }
                }
            }
        }

        private void HelpButton_Click(object sender, EventArgs e)
        {
            NavigationService.Navigate(new Uri("/HelpPage.xaml", UriKind.Relative));
        }

        private void blurGeneratorWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            int   radius = (e.Argument as BlurGenTaskData).radius;
            int   width  = (e.Argument as BlurGenTaskData).width;
            int   height = (e.Argument as BlurGenTaskData).height;
            int[] pixels = (e.Argument as BlurGenTaskData).pixels;
            
            int[] blur_pixels = pixels.Clone() as int[];

            int[] tab   = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
            int   alpha = (radius < 1) ? 16 : (radius > 17) ? 1 : tab[radius - 1];

            int   r1   = 0;
            int   r2   = height - 1;
            int   c1   = 0;
            int   c2   = width - 1;
            int[] rgba = new int[4];

            for (int col = c1; col <= c2; col++)
            {
                int s = r1 * width + col;

                for (int i = 0; i < 4; i++)
                {
                    rgba[i] = ((blur_pixels[s] >> (i * 8)) & 0xFF) << 4;
                }

                s += width;

                for (int j = r1; j < r2; j++, s += width)
                {
                    int p = 0;

                    for (int i = 0; i < 4; i++)
                    {
                        p = p | ((((rgba[i] += ((((blur_pixels[s] >> (i * 8)) & 0xFF) << 4) - rgba[i]) * alpha / 16) >> 4) & 0xFF) << (i * 8));
                    }

                    blur_pixels[s] = p;
                }
            }

            for (int row = r1; row <= r2; row++)
            {
                int s = row * width + c1;

                for (int i = 0; i < 4; i++)
                {
                    rgba[i] = ((blur_pixels[s] >> (i * 8)) & 0xFF) << 4;
                }

                s++;

                for (int j = c1; j < c2; j++, s++)
                {
                    int p = 0;

                    for (int i = 0; i < 4; i++)
                    {
                        p = p | ((((rgba[i] += ((((blur_pixels[s] >> (i * 8)) & 0xFF) << 4) - rgba[i]) * alpha / 16) >> 4) & 0xFF) << (i * 8));
                    }

                    blur_pixels[s] = p;
                }
            }

            for (int col = c1; col <= c2; col++)
            {
                int s = r2 * width + col;

                for (int i = 0; i < 4; i++)
                {
                    rgba[i] = ((blur_pixels[s] >> (i * 8)) & 0xFF) << 4;
                }

                s -= width;

                for (int j = r1; j < r2; j++, s -= width)
                {
                    int p = 0;

                    for (int i = 0; i < 4; i++)
                    {
                        p = p | ((((rgba[i] += ((((blur_pixels[s] >> (i * 8)) & 0xFF) << 4) - rgba[i]) * alpha / 16) >> 4) & 0xFF) << (i * 8));
                    }

                    blur_pixels[s] = p;
                }
            }

            for (int row = r1; row <= r2; row++)
            {
                int s = row * width + c2;

                for (int i = 0; i < 4; i++)
                {
                    rgba[i] = ((blur_pixels[s] >> (i * 8)) & 0xFF) << 4;
                }

                s--;

                for (int j = c1; j < c2; j++, s--)
                {
                    int p = 0;

                    for (int i = 0; i < 4; i++)
                    {
                        p = p | ((((rgba[i] += ((((blur_pixels[s] >> (i * 8)) & 0xFF) << 4) - rgba[i]) * alpha / 16) >> 4) & 0xFF) << (i * 8));
                    }

                    blur_pixels[s] = p;
                }
            }

            BlurGenTaskData task_data = new BlurGenTaskData();

            task_data.radius = radius;
            task_data.width  = width;
            task_data.height = height;
            task_data.pixels = blur_pixels;

            e.Result = task_data;
        }

        private void blurGeneratorWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            this.GenerationProgressIndicator.IsVisible = false;

            if (!e.Cancelled && e.Error == null)
            {
                WriteableBitmap bitmap = new WriteableBitmap((e.Result as BlurGenTaskData).width, (e.Result as BlurGenTaskData).height);

                (e.Result as BlurGenTaskData).pixels.CopyTo(bitmap.Pixels, 0);

                this.previewBitmap = bitmap;

                this.PreviewImage.Source = this.previewBitmap;
            }

            if (this.restartBlurGenerator)
            {
                StartBlurGenerator();

                this.restartBlurGenerator = false;
            }
        }

        private void photoChooserTask_Completed(object sender, PhotoResult e)
        {
            if (e != null && e.TaskResult == TaskResult.OK && e.ChosenPhoto != null)
            {
                WriteableBitmap bitmap = new WriteableBitmap(0, 0);

                bitmap.SetSource(e.ChosenPhoto);

                LoadImage(bitmap);
            }
            else
            {
                this.loadImageCancelled = true;
            }
        }

        private void GaussianRadiusSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            this.gaussianRadius = (int)e.NewValue;

            if (this.blurGeneratorWorker != null)
            {
                if (this.blurGeneratorWorker.IsBusy)
                {
                    this.restartBlurGenerator = true;
                }
                else
                {
                    StartBlurGenerator();
                }
            }
        }
    }
}