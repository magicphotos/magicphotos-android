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
    public partial class PixelatePreviewPage : PhoneApplicationPage
    {
        private const int MAX_LOADED_WIDTH  = 2048,
                          MAX_LOADED_HEIGHT = 2048;

        private const double PREVIEW_MPIX_LIMIT = 0.5;

        class PixelateGenTaskData
        {
            public int   pix_denom;
            public int   width, height;
            public int[] pixels;
        }

        private bool             loadImageOnLayoutUpdate,
                                 loadImageCancelled,
                                 pageNavigationComplete,
                                 restartPixelateGenerator;
        private int              pixDenom;
        private WriteableBitmap  loadedBitmap,
                                 scaledBitmap,
                                 previewBitmap;
        private BackgroundWorker pixelateGeneratorWorker;
        private PhotoChooserTask photoChooserTask;

        public PixelatePreviewPage()
        {
            InitializeComponent();

            this.loadImageOnLayoutUpdate  = true;
            this.loadImageCancelled       = false;
            this.pageNavigationComplete   = false;
            this.restartPixelateGenerator = false;
            this.pixDenom                 = (int)this.PixDenomSlider.Value;
            this.loadedBitmap             = null;
            this.scaledBitmap             = null;
            this.previewBitmap            = null;

            this.pixelateGeneratorWorker                     = new BackgroundWorker();
            this.pixelateGeneratorWorker.DoWork             += new DoWorkEventHandler(pixelateGeneratorWorker_DoWork);
            this.pixelateGeneratorWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(pixelateGeneratorWorker_RunWorkerCompleted);

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

        private void StartPixelateGenerator()
        {
            if (this.scaledBitmap != null)
            {
                try
                {
                    PixelateGenTaskData task_data = new PixelateGenTaskData();

                    task_data.pix_denom = this.pixDenom;
                    task_data.width     = this.scaledBitmap.PixelWidth;
                    task_data.height    = this.scaledBitmap.PixelHeight;
                    task_data.pixels    = this.scaledBitmap.Pixels;

                    this.pixelateGeneratorWorker.RunWorkerAsync(task_data);

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

                    StartPixelateGenerator();
                }
            }
        }

        private void PixelatePreviewPage_LayoutUpdated(object sender, EventArgs e)
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
                                WriteableBitmap bitmap = PictureDecoder.DecodeJpeg(stream, MAX_LOADED_WIDTH, MAX_LOADED_HEIGHT);

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

                        NavigationService.Navigate(new Uri(string.Format("/PixelatePage.xaml?pix_denom={0}", Uri.EscapeUriString(this.pixDenom.ToString())), UriKind.Relative));
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

        private void pixelateGeneratorWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            int   pix_denom = (e.Argument as PixelateGenTaskData).pix_denom;
            int   width     = (e.Argument as PixelateGenTaskData).width;
            int   height    = (e.Argument as PixelateGenTaskData).height;
            int[] pixels    = (e.Argument as PixelateGenTaskData).pixels;

            int[] pixelated_pixels = pixels.Clone() as int[];
            int   pix_size         = width > height ? width / pix_denom : height / pix_denom;

            if (pix_size != 0)
            {
                for (int i = 0; i < width / pix_size + 1; i++)
                {
                    for (int j = 0; j < height / pix_size + 1; j++)
                    {
                        int avg_r  = 0;
                        int avg_g  = 0;
                        int avg_b  = 0;
                        int pixcnt = 0;

                        for (int x = i * pix_size; x < (i + 1) * pix_size && x < width; x++)
                        {
                            for (int y = j * pix_size; y < (j + 1) * pix_size && y < height; y++)
                            {
                                byte[] color = BitConverter.GetBytes(pixelated_pixels[y * width + x]);

                                avg_r += color[0];
                                avg_g += color[1];
                                avg_b += color[2];

                                pixcnt++;
                            }
                        }

                        if (pixcnt != 0)
                        {
                            avg_r = avg_r / pixcnt;
                            avg_g = avg_g / pixcnt;
                            avg_b = avg_b / pixcnt;

                            for (int x = i * pix_size; x < (i + 1) * pix_size && x < width; x++)
                            {
                                for (int y = j * pix_size; y < (j + 1) * pix_size && y < height; y++)
                                {
                                    byte[] color = BitConverter.GetBytes(pixelated_pixels[y * width + x]);

                                    pixelated_pixels[y * width + x] = (color[3] << 24) | (avg_b << 16) | (avg_g << 8) | avg_r;
                                }
                            }
                        }
                    }
                }
            }

            PixelateGenTaskData task_data = new PixelateGenTaskData();

            task_data.pix_denom = pix_denom;
            task_data.width     = width;
            task_data.height    = height;
            task_data.pixels    = pixelated_pixels;

            e.Result = task_data;
        }

        private void pixelateGeneratorWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            this.GenerationProgressIndicator.IsVisible = false;

            if (!e.Cancelled && e.Error == null)
            {
                WriteableBitmap bitmap = new WriteableBitmap((e.Result as PixelateGenTaskData).width, (e.Result as PixelateGenTaskData).height);

                (e.Result as PixelateGenTaskData).pixels.CopyTo(bitmap.Pixels, 0);

                this.previewBitmap = bitmap;

                this.PreviewImage.Source = this.previewBitmap;
            }

            if (this.restartPixelateGenerator)
            {
                StartPixelateGenerator();

                this.restartPixelateGenerator = false;
            }
        }

        private void photoChooserTask_Completed(object sender, PhotoResult e)
        {
            if (e != null && e.TaskResult == TaskResult.OK && e.ChosenPhoto != null)
            {
                WriteableBitmap bitmap = PictureDecoder.DecodeJpeg(e.ChosenPhoto, MAX_LOADED_WIDTH, MAX_LOADED_HEIGHT);

                LoadImage(bitmap);
            }
            else
            {
                this.loadImageCancelled = true;
            }
        }

        private void PixDenomSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            this.pixDenom = (int)e.NewValue;

            if (this.pixelateGeneratorWorker != null)
            {
                if (this.pixelateGeneratorWorker.IsBusy)
                {
                    this.restartPixelateGenerator = true;
                }
                else
                {
                    StartPixelateGenerator();
                }
            }
        }
    }
}