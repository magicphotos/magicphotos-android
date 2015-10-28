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
using System.Windows.Resources;
using System.Windows.Shapes;
using Microsoft.Phone;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Info;
using Microsoft.Phone.Tasks;
using Microsoft.Phone.Shell;
using Microsoft.Xna.Framework.Media;

namespace MagicPhotos
{
    public partial class DecolorizePage : PhoneApplicationPage
    {
        private const int MODE_NONE     = 0,
                          MODE_SCROLL   = 1,
                          MODE_ORIGINAL = 2,
                          MODE_EFFECTED = 3;

        private const int MAX_LOADED_WIDTH  = 2048,
                          MAX_LOADED_HEIGHT = 2048;

        private const int MAX_IMAGE_WIDTH  = 2800,
                          MAX_IMAGE_HEIGHT = 2800;

        private const int HELPER_POINT_RADIUS = 3;

        private const int DEFAULT_BRUSH_RADIUS = 24,
                          UNDO_DEPTH           = 4;

        private const double DEFAULT_BRUSH_OPACITY = 0.75;

        class DecolorizeGenTaskData
        {
            public int   width, height;
            public int[] pixels;
        }

        private bool                  loadImageOnLayoutUpdate,
                                      loadImageCancelled,
                                      pageNavigationComplete,
                                      editedImageChanged;
        private int                   selectedMode,
                                      brushRadius;
        private double                currentScale,
                                      initialScale,
                                      brushOpacity;
        private List<int[]>           undoStack;
        private WriteableBitmap       loadedBitmap,
                                      editedBitmap,
                                      originalBitmap,
                                      effectedBitmap,
                                      helperBitmap,
                                      brushTemplateBitmap,
                                      brushBitmap,
                                      brushPreviewBitmap;
        private BackgroundWorker      decolorizeGeneratorWorker;
        private PhotoChooserTask      photoChooserTask;
        private MarketplaceDetailTask marketplaceDetailTask;

        public DecolorizePage()
        {
            InitializeComponent();

            this.loadImageOnLayoutUpdate = true;
            this.loadImageCancelled      = false;
            this.pageNavigationComplete  = false;
            this.editedImageChanged      = false;
            this.selectedMode            = MODE_NONE;
            this.currentScale            = 1.0;
            this.initialScale            = 1.0;
            this.undoStack               = new List<int[]>();
            this.loadedBitmap            = null;
            this.editedBitmap            = null;
            this.originalBitmap          = null;
            this.effectedBitmap          = null;
            this.helperBitmap            = null;
            this.brushBitmap             = null;

            if (!IsolatedStorageSettings.ApplicationSettings.TryGetValue<int>("BrushRadius", out this.brushRadius))
            {
                this.brushRadius = DEFAULT_BRUSH_RADIUS;
            }
            if (!IsolatedStorageSettings.ApplicationSettings.TryGetValue<double>("BrushOpacity", out this.brushOpacity))
            {
                this.brushOpacity = DEFAULT_BRUSH_OPACITY;
            }

            this.BrushRadiusSlider.Value  = this.brushRadius;
            this.BrushOpacitySlider.Value = this.brushOpacity;

            this.brushTemplateBitmap = new WriteableBitmap(this.brushRadius * 2, this.brushRadius * 2);

            for (int x = 0; x < this.brushTemplateBitmap.PixelWidth; x++)
            {
                for (int y = 0; y < this.brushTemplateBitmap.PixelHeight; y++)
                {
                    double r = Math.Sqrt(Math.Pow(x - this.brushRadius, 2) + Math.Pow(y - this.brushRadius, 2));

                    if (r <= this.brushRadius)
                    {
                        if (r <= this.brushRadius * this.brushOpacity)
                        {
                            this.brushTemplateBitmap.SetPixel(x, y, (0xFF << 24) | 0xFFFFFF);
                        }
                        else
                        {
                            this.brushTemplateBitmap.SetPixel(x, y, ((byte)(0xFF * (this.brushRadius - r) / (this.brushRadius * (1.0 - this.brushOpacity))) << 24) | 0xFFFFFF);
                        }
                    }
                    else
                    {
                        this.brushTemplateBitmap.SetPixel(x, y, (0x00 << 24) | 0xFFFFFF);
                    }
                }
            }

            WriteableBitmap brush_preview = new WriteableBitmap(this.brushTemplateBitmap.PixelWidth,     this.brushTemplateBitmap.PixelHeight);
            this.brushPreviewBitmap       = new WriteableBitmap((int)this.BrushRadiusSlider.Maximum * 2, (int)this.BrushRadiusSlider.Maximum * 2);

            Rect brt_rect = new Rect(0, 0, this.brushTemplateBitmap.PixelWidth, this.brushTemplateBitmap.PixelHeight);
            Rect pre_rect = new Rect(((int)this.brushPreviewBitmap.PixelWidth  - this.brushTemplateBitmap.PixelWidth)  / 2,
                                     ((int)this.brushPreviewBitmap.PixelHeight - this.brushTemplateBitmap.PixelHeight) / 2,
                                     this.brushTemplateBitmap.PixelWidth,
                                     this.brushTemplateBitmap.PixelHeight);

            brush_preview.Clear(Colors.Red);
            brush_preview.Blit(brt_rect, this.brushTemplateBitmap, brt_rect, WriteableBitmapExtensions.BlendMode.Mask);

            this.brushPreviewBitmap.Clear(Color.FromArgb(0x00, 0x00, 0x00, 0x00));
            this.brushPreviewBitmap.Blit(pre_rect, brush_preview, brt_rect, WriteableBitmapExtensions.BlendMode.None);

            this.BrushPreviewImage.Source = this.brushPreviewBitmap;

            this.decolorizeGeneratorWorker                     = new BackgroundWorker();
            this.decolorizeGeneratorWorker.DoWork             += new DoWorkEventHandler(decolorizeGeneratorWorker_DoWork);
            this.decolorizeGeneratorWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(decolorizeGeneratorWorker_RunWorkerCompleted);

            this.photoChooserTask            = new PhotoChooserTask();
            this.photoChooserTask.ShowCamera = true;
            this.photoChooserTask.Completed += new EventHandler<PhotoResult>(photoChooserTask_Completed);

            this.marketplaceDetailTask                   = new MarketplaceDetailTask();
#if DEBUG_TRIAL
            this.marketplaceDetailTask.ContentType       = MarketplaceContentType.Applications;
            this.marketplaceDetailTask.ContentIdentifier = "ae587193-24c3-49ff-8743-88f5f05907c1";
#endif

            ApplicationBarIconButton button;

            button        = new ApplicationBarIconButton(new Uri("/Images/save.png", UriKind.Relative));
            button.Text   = AppResources.ApplicationBarButtonSaveText;
            button.Click += new EventHandler(SaveButton_Click);

            this.ApplicationBar.Buttons.Add(button);

            button        = new ApplicationBarIconButton(new Uri("/Images/settings.png", UriKind.Relative));
            button.Text   = AppResources.ApplicationBarButtonSettingsText;
            button.Click += new EventHandler(SettingsButton_Click);

            this.ApplicationBar.Buttons.Add(button);

            button        = new ApplicationBarIconButton(new Uri("/Images/help.png", UriKind.Relative));
            button.Text   = AppResources.ApplicationBarButtonHelpText;
            button.Click += new EventHandler(HelpButton_Click);

            this.ApplicationBar.Buttons.Add(button);

            if ((System.Windows.Visibility)App.Current.Resources["PhoneDarkThemeVisibility"] == System.Windows.Visibility.Visible)
            {
                this.UndoButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri("/Images/dark/undo.png", UriKind.Relative)) };
            }
            else
            {
                this.UndoButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri("/Images/light/undo.png", UriKind.Relative)) };
            }

            UpdateModeButtons();
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

        private void UpdateModeButtons()
        {
            string img_dir;

            if ((System.Windows.Visibility)App.Current.Resources["PhoneDarkThemeVisibility"] == System.Windows.Visibility.Visible)
            {
                img_dir = "/Images/dark";
            }
            else
            {
                img_dir = "/Images/light";
            }

            if (this.selectedMode == MODE_SCROLL)
            {
                this.ScrollModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_scroll_selected.png", UriKind.Relative)) };
            }
            else
            {
                this.ScrollModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_scroll.png", UriKind.Relative)) };
            }

            if (this.selectedMode == MODE_ORIGINAL)
            {
                this.OriginalModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_original_selected.png", UriKind.Relative)) };
            }
            else
            {
                this.OriginalModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_original.png", UriKind.Relative)) };
            }

            if (this.selectedMode == MODE_EFFECTED)
            {
                this.EffectedModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_effected_selected.png", UriKind.Relative)) };
            }
            else
            {
                this.EffectedModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_effected.png", UriKind.Relative)) };
            }
        }

        private void MoveHelper(Point touch_point)
        {
            if (touch_point.Y < this.HelperBorder.Height * 1.5)
            {
                if (touch_point.X < this.HelperBorder.Width * 1.5)
                {
                    this.HelperBorder.HorizontalAlignment = System.Windows.HorizontalAlignment.Right;
                }
                else if (touch_point.X > this.EditorGrid.ActualWidth - this.HelperBorder.Width * 1.5)
                {
                    this.HelperBorder.HorizontalAlignment = System.Windows.HorizontalAlignment.Left;
                }
            }
        }

        private void UpdateHelper(bool visible, Point touch_point)
        {
            if (visible)
            {
                int width  = Math.Min((int)(this.HelperImage.Width  / this.currentScale), this.editedBitmap.PixelWidth);
                int height = Math.Min((int)(this.HelperImage.Height / this.currentScale), this.editedBitmap.PixelHeight);

                int x = Math.Min(Math.Max(0, (int)(touch_point.X - width  / 2)), this.editedBitmap.PixelWidth  - width);
                int y = Math.Min(Math.Max(0, (int)(touch_point.Y - height / 2)), this.editedBitmap.PixelHeight - height);

                this.helperBitmap = this.editedBitmap.Crop(x, y, width, height);

                int helper_point_radius = Math.Max(1, (int)(HELPER_POINT_RADIUS / this.currentScale));

                this.helperBitmap.FillEllipseCentered(width / 2, height / 2, helper_point_radius, helper_point_radius, 0x00FFFFFF);

                this.HelperImage.Source = this.helperBitmap;

                this.HelperBorder.Visibility = System.Windows.Visibility.Visible;
            }
            else
            {
                this.HelperBorder.Visibility = System.Windows.Visibility.Collapsed;
            }
        }

        private void LoadImage(WriteableBitmap bitmap)
        {
            if (bitmap.PixelWidth != 0 && bitmap.PixelHeight != 0)
            {
                this.loadedBitmap = bitmap;

                DecolorizeGenTaskData task_data = new DecolorizeGenTaskData();

                task_data.width  = bitmap.PixelWidth;
                task_data.height = bitmap.PixelHeight;
                task_data.pixels = bitmap.Pixels;

                this.decolorizeGeneratorWorker.RunWorkerAsync(task_data);

                this.GenerationProgressIndicator.IsVisible = true;
            }
        }

        private void SaveUndoImage()
        {
            if (this.editedBitmap.Pixels.Length > 0)
            {
                int[] pixels = new int[this.editedBitmap.Pixels.Length];

                this.editedBitmap.Pixels.CopyTo(pixels, 0);

                this.undoStack.Add(pixels);

                if (this.undoStack.Count > UNDO_DEPTH)
                {
                    for (int i = 0; i < this.undoStack.Count - UNDO_DEPTH; i++)
                    {
                        this.undoStack.RemoveAt(0);
                    }
                }
            }
        }

        private void ChangeBitmap(Point touch_point)
        {
            if (this.selectedMode == MODE_ORIGINAL || this.selectedMode == MODE_EFFECTED)
            {
                int width  = Math.Min(this.brushBitmap.PixelWidth,  this.editedBitmap.PixelWidth);
                int height = Math.Min(this.brushBitmap.PixelHeight, this.editedBitmap.PixelHeight);

                Rect img_rect = new Rect();

                img_rect.X      = Math.Min(Math.Max(0, touch_point.X - width  / 2), this.editedBitmap.PixelWidth  - width);
                img_rect.Y      = Math.Min(Math.Max(0, touch_point.Y - height / 2), this.editedBitmap.PixelHeight - height);
                img_rect.Width  = width;
                img_rect.Height = height;

                Rect brh_rect = new Rect(0, 0, width, height);

                WriteableBitmap brush_bitmap = new WriteableBitmap(width, height);

                if (this.selectedMode == MODE_ORIGINAL)
                {
                    brush_bitmap.Blit(brh_rect, this.originalBitmap, img_rect, WriteableBitmapExtensions.BlendMode.None);
                }
                else
                {
                    brush_bitmap.Blit(brh_rect, this.effectedBitmap, img_rect, WriteableBitmapExtensions.BlendMode.None);
                }

                brush_bitmap.Blit     (brh_rect, this.brushBitmap, brh_rect, WriteableBitmapExtensions.BlendMode.Mask);
                this.editedBitmap.Blit(img_rect, brush_bitmap,     brh_rect, WriteableBitmapExtensions.BlendMode.Alpha);

                this.EditorImage.Source = this.editedBitmap;
            }
        }

        private void DecolorizePage_LayoutUpdated(object sender, EventArgs e)
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

        private void DecolorizePage_OrientationChanged(object sender, OrientationChangedEventArgs e)
        {
            double x = this.EditorImageTransform.TranslateX;
            double y = this.EditorImageTransform.TranslateY;

            if (x < this.EditorGrid.ActualWidth - this.EditorImage.ActualWidth * this.currentScale)
            {
                x = this.EditorGrid.ActualWidth - this.EditorImage.ActualWidth * this.currentScale;
            }
            if (x > 0.0)
            {
                x = 0.0;
            }

            if (y < this.EditorGrid.ActualHeight - this.EditorImage.ActualHeight * this.currentScale)
            {
                y = this.EditorGrid.ActualHeight - this.EditorImage.ActualHeight * this.currentScale;
            }
            if (y > 0.0)
            {
                y = 0.0;
            }

            this.EditorImageTransform.TranslateX = x;
            this.EditorImageTransform.TranslateY = y;
        }

        private void DecolorizePage_BackKeyPress(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (this.BrushSettingsGrid.Visibility == System.Windows.Visibility.Visible)
            {
                this.BrushSettingsGrid.Visibility = System.Windows.Visibility.Collapsed;

                e.Cancel = true;
            }
            else if (this.editedImageChanged)
            {
                MessageBoxResult result = MessageBoxResult.None;

                try
                {
                    result = MessageBox.Show(AppResources.MessageBoxMessageUnsavedImageQuestion, AppResources.MessageBoxHeaderWarning, MessageBoxButton.OKCancel);
                }
                catch (Exception)
                {
                    result = MessageBoxResult.None;
                }

                if (result == MessageBoxResult.Cancel)
                {
                    e.Cancel = true;
                }
            }
        }

        private void UndoButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.undoStack.Count > 0)
            {
                this.editedImageChanged = true;

                int[] pixels = this.undoStack.ElementAt(this.undoStack.Count - 1);

                this.undoStack.RemoveAt(this.undoStack.Count - 1);

                if (pixels.Length == this.editedBitmap.Pixels.Length)
                {
                    pixels.CopyTo(this.editedBitmap.Pixels, 0);
                }

                this.EditorImage.Source = this.editedBitmap;
            }
        }

        private void ScrollModeButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.selectedMode != MODE_NONE)
            {
                this.selectedMode = MODE_SCROLL;

                UpdateModeButtons();
            }
        }

        private void OriginalModeButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.selectedMode != MODE_NONE)
            {
                this.selectedMode = MODE_ORIGINAL;

                UpdateModeButtons();
            }
        }

        private void EffectedModeButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.selectedMode != MODE_NONE)
            {
                this.selectedMode = MODE_EFFECTED;

                UpdateModeButtons();
            }
        }

        private void SaveButton_Click(object sender, EventArgs e)
        {
            if (this.editedBitmap != null)
            {
                if ((Application.Current as App).TrialMode)
                {
                    MessageBoxResult result = MessageBoxResult.None;

                    try
                    {
                        result = MessageBox.Show(AppResources.MessageBoxMessageTrialVersionQuestion, AppResources.MessageBoxHeaderInfo, MessageBoxButton.OKCancel);
                    }
                    catch (Exception)
                    {
                        result = MessageBoxResult.None;
                    }

                    if (result == MessageBoxResult.OK)
                    {
                        try
                        {
                            this.marketplaceDetailTask.Show();
                        }
                        catch (Exception ex)
                        {
                            try
                            {
                                MessageBox.Show(AppResources.MessageBoxMessageMarketplaceOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                            }
                            catch (Exception)
                            {
                                // Ignore
                            }
                        }
                    }
                }
                else
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
                                this.editedBitmap.SaveJpeg(stream, this.editedBitmap.PixelWidth, this.editedBitmap.PixelHeight, 0, 100);
                            }

                            using (IsolatedStorageFileStream stream = store.OpenFile(file_name, FileMode.Open, FileAccess.Read))
                            {
                                using (MediaLibrary library = new MediaLibrary())
                                {
                                    library.SavePicture(file_name, stream);
                                }
                            }

                            store.DeleteFile(file_name);
                        }

                        this.editedImageChanged = false;

                        try
                        {
                            MessageBox.Show(AppResources.MessageBoxMessageImageSavedInfo, AppResources.MessageBoxHeaderInfo, MessageBoxButton.OK);
                        }
                        catch (Exception)
                        {
                            // Ignore
                        }
                    }
                    catch (Exception ex)
                    {
                        try
                        {
                            MessageBox.Show(AppResources.MessageBoxMessageImageSaveError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                        }
                        catch (Exception)
                        {
                            // Ignore
                        }
                    }
                }
            }
        }

        private void SettingsButton_Click(object sender, EventArgs e)
        {
            if (this.BrushSettingsGrid.Visibility == System.Windows.Visibility.Collapsed)
            {
                this.BrushSettingsGrid.Visibility = System.Windows.Visibility.Visible;
            }
            else
            {
                this.BrushSettingsGrid.Visibility = System.Windows.Visibility.Collapsed;
            }
        }

        private void HelpButton_Click(object sender, EventArgs e)
        {
            NavigationService.Navigate(new Uri("/HelpPage.xaml", UriKind.Relative));
        }

        private void decolorizeGeneratorWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            int   width  = (e.Argument as DecolorizeGenTaskData).width;
            int   height = (e.Argument as DecolorizeGenTaskData).height;
            int[] pixels = (e.Argument as DecolorizeGenTaskData).pixels;

            int[] grayscale_pixels = pixels.Clone() as int[];

            for (int i = 0; i < grayscale_pixels.Length; i++)
            {
                byte[] color = BitConverter.GetBytes(grayscale_pixels[i]);

                byte gray = (byte)((color[0] * 11 + color[1] * 16 + color[2] * 5) / 32);

                grayscale_pixels[i] = (color[3] << 24) | (gray << 16) | (gray << 8) | gray;
            }

            DecolorizeGenTaskData task_data = new DecolorizeGenTaskData();

            task_data.width  = width;
            task_data.height = height;
            task_data.pixels = grayscale_pixels;

            e.Result = task_data;
        }

        private void decolorizeGeneratorWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            this.GenerationProgressIndicator.IsVisible = false;

            if (!e.Cancelled && e.Error == null)
            {
                WriteableBitmap bitmap = new WriteableBitmap((e.Result as DecolorizeGenTaskData).width, (e.Result as DecolorizeGenTaskData).height);

                (e.Result as DecolorizeGenTaskData).pixels.CopyTo(bitmap.Pixels, 0);

                this.editedImageChanged = true;
                this.selectedMode       = MODE_SCROLL;

                this.undoStack.Clear();

                this.originalBitmap = this.loadedBitmap.Clone();
                this.effectedBitmap = bitmap.Clone();
                this.editedBitmap   = bitmap.Clone();

                this.loadedBitmap = null;

                if (this.editedBitmap.PixelWidth > this.editedBitmap.PixelHeight)
                {
                    this.currentScale = this.EditorGrid.ActualWidth / this.editedBitmap.PixelWidth;
                }
                else
                {
                    this.currentScale = this.EditorGrid.ActualHeight / this.editedBitmap.PixelHeight;
                }

                this.EditorImage.Visibility = System.Windows.Visibility.Visible;
                this.EditorImage.Source     = this.editedBitmap;

                this.EditorImageGrid.Width  = MAX_IMAGE_WIDTH;
                this.EditorImageGrid.Height = MAX_IMAGE_HEIGHT;

                this.EditorImageTransform.TranslateX = 0.0;
                this.EditorImageTransform.TranslateY = 0.0;
                this.EditorImageTransform.ScaleX     = this.currentScale;
                this.EditorImageTransform.ScaleY     = this.currentScale;

                int brush_width = Math.Max(1, Math.Min(Math.Min((int)(this.brushRadius / this.currentScale) * 2, this.editedBitmap.PixelWidth), this.editedBitmap.PixelHeight));

                this.brushBitmap = this.brushTemplateBitmap.Resize(brush_width, brush_width, WriteableBitmapExtensions.Interpolation.NearestNeighbor);

                UpdateModeButtons();
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

        private void EditorGrid_MouseEnter(object sender, MouseEventArgs e)
        {
            MoveHelper(e.GetPosition(this.EditorGrid));
        }

        private void EditorGrid_MouseMove(object sender, MouseEventArgs e)
        {
            MoveHelper(e.GetPosition(this.EditorGrid));
        }

        private void EditorGrid_MouseLeave(object sender, MouseEventArgs e)
        {
            MoveHelper(e.GetPosition(this.EditorGrid));
        }

        private void EditorImage_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            this.EditorImage.CaptureMouse();

            if (this.selectedMode == MODE_ORIGINAL || this.selectedMode == MODE_EFFECTED)
            {
                this.editedImageChanged = true;

                SaveUndoImage();

                ChangeBitmap(e.GetPosition(this.EditorImage));

                UpdateHelper(true, e.GetPosition(this.EditorImage));
            }
        }

        private void EditorImage_MouseMove(object sender, MouseEventArgs e)
        {
            if (this.selectedMode == MODE_ORIGINAL || this.selectedMode == MODE_EFFECTED)
            {
                ChangeBitmap(e.GetPosition(this.EditorImage));

                UpdateHelper(true, e.GetPosition(this.EditorImage));
            }
        }

        private void EditorImage_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            this.EditorImage.ReleaseMouseCapture();

            if (this.selectedMode == MODE_ORIGINAL || this.selectedMode == MODE_EFFECTED)
            {
                UpdateHelper(false, e.GetPosition(this.EditorImage));
            }
        }

        private void EditorImage_MouseLeave(object sender, MouseEventArgs e)
        {
            if (this.selectedMode == MODE_ORIGINAL || this.selectedMode == MODE_EFFECTED)
            {
                UpdateHelper(false, e.GetPosition(this.EditorImage));
            }
        }

        private void EditorImage_DragDelta(object sender, DragDeltaGestureEventArgs e)
        {
            if (this.selectedMode == MODE_SCROLL)
            {
                e.Handled = true;

                double x = this.EditorImageTransform.TranslateX + e.HorizontalChange;
                double y = this.EditorImageTransform.TranslateY + e.VerticalChange;

                if (x < this.EditorGrid.ActualWidth - this.EditorImage.ActualWidth * this.currentScale)
                {
                    x = this.EditorGrid.ActualWidth - this.EditorImage.ActualWidth * this.currentScale;
                }
                if (x > 0.0)
                {
                    x = 0.0;
                }

                if (y < this.EditorGrid.ActualHeight - this.EditorImage.ActualHeight * this.currentScale)
                {
                    y = this.EditorGrid.ActualHeight - this.EditorImage.ActualHeight * this.currentScale;
                }
                if (y > 0.0)
                {
                    y = 0.0;
                }

                this.EditorImageTransform.TranslateX = x;
                this.EditorImageTransform.TranslateY = y;
            }
        }

        private void EditorImage_PinchStarted(object sender, PinchStartedGestureEventArgs e)
        {
            if (this.selectedMode == MODE_SCROLL)
            {
                e.Handled = true;

                this.initialScale = this.currentScale;
            }
        }

        private void EditorImage_PinchDelta(object sender, PinchGestureEventArgs e)
        {
            if (this.selectedMode == MODE_SCROLL)
            {
                e.Handled = true;

                double scale  = this.initialScale             * e.DistanceRatio;
                double width  = this.editedBitmap.PixelWidth  * scale;
                double height = this.editedBitmap.PixelHeight * scale;

                if ((width >= this.EditorGrid.ActualWidth || height >= this.EditorGrid.ActualHeight) &&
                    (width <= MAX_IMAGE_WIDTH             && height <= MAX_IMAGE_HEIGHT))
                {
                    this.currentScale = scale;

                    double x = this.EditorImageTransform.TranslateX;
                    double y = this.EditorImageTransform.TranslateY;

                    if (x < this.EditorGrid.ActualWidth - this.EditorImage.ActualWidth * this.currentScale)
                    {
                        x = this.EditorGrid.ActualWidth - this.EditorImage.ActualWidth * this.currentScale;
                    }
                    if (x > 0.0)
                    {
                        x = 0.0;
                    }

                    if (y < this.EditorGrid.ActualHeight - this.EditorImage.ActualHeight * this.currentScale)
                    {
                        y = this.EditorGrid.ActualHeight - this.EditorImage.ActualHeight * this.currentScale;
                    }
                    if (y > 0.0)
                    {
                        y = 0.0;
                    }

                    this.EditorImageTransform.TranslateX = x;
                    this.EditorImageTransform.TranslateY = y;
                    this.EditorImageTransform.ScaleX     = this.currentScale;
                    this.EditorImageTransform.ScaleY     = this.currentScale;

                    int brush_width = Math.Max(1, Math.Min(Math.Min((int)(this.brushRadius / this.currentScale) * 2, this.editedBitmap.PixelWidth), this.editedBitmap.PixelHeight));

                    this.brushBitmap = this.brushTemplateBitmap.Resize(brush_width, brush_width, WriteableBitmapExtensions.Interpolation.NearestNeighbor);
                }
            }
        }

        private void BrushRadiusSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            this.brushRadius = (int)e.NewValue;

            this.brushTemplateBitmap = new WriteableBitmap(this.brushRadius * 2, this.brushRadius * 2);

            for (int x = 0; x < this.brushTemplateBitmap.PixelWidth; x++)
            {
                for (int y = 0; y < this.brushTemplateBitmap.PixelHeight; y++)
                {
                    double r = Math.Sqrt(Math.Pow(x - this.brushRadius, 2) + Math.Pow(y - this.brushRadius, 2));

                    if (r <= this.brushRadius)
                    {
                        if (r <= this.brushRadius * this.brushOpacity)
                        {
                            this.brushTemplateBitmap.SetPixel(x, y, (0xFF << 24) | 0xFFFFFF);
                        }
                        else
                        {
                            this.brushTemplateBitmap.SetPixel(x, y, ((byte)(0xFF * (this.brushRadius - r) / (this.brushRadius * (1.0 - this.brushOpacity))) << 24) | 0xFFFFFF);
                        }
                    }
                    else
                    {
                        this.brushTemplateBitmap.SetPixel(x, y, (0x00 << 24) | 0xFFFFFF);
                    }
                }
            }

            if (this.editedBitmap != null)
            {
                int brush_width = Math.Max(1, Math.Min(Math.Min((int)(this.brushRadius / this.currentScale) * 2, this.editedBitmap.PixelWidth), this.editedBitmap.PixelHeight));

                this.brushBitmap = this.brushTemplateBitmap.Resize(brush_width, brush_width, WriteableBitmapExtensions.Interpolation.NearestNeighbor);
            }

            if (this.BrushRadiusSlider != null && this.BrushPreviewImage != null)
            {
                WriteableBitmap brush_preview = new WriteableBitmap(this.brushTemplateBitmap.PixelWidth,     this.brushTemplateBitmap.PixelHeight);
                this.brushPreviewBitmap       = new WriteableBitmap((int)this.BrushRadiusSlider.Maximum * 2, (int)this.BrushRadiusSlider.Maximum * 2);

                Rect brt_rect = new Rect(0, 0, this.brushTemplateBitmap.PixelWidth, this.brushTemplateBitmap.PixelHeight);
                Rect pre_rect = new Rect(((int)this.brushPreviewBitmap.PixelWidth  - this.brushTemplateBitmap.PixelWidth)  / 2,
                                         ((int)this.brushPreviewBitmap.PixelHeight - this.brushTemplateBitmap.PixelHeight) / 2,
                                         this.brushTemplateBitmap.PixelWidth,
                                         this.brushTemplateBitmap.PixelHeight);

                brush_preview.Clear(Colors.Red);
                brush_preview.Blit(brt_rect, this.brushTemplateBitmap, brt_rect, WriteableBitmapExtensions.BlendMode.Mask);

                this.brushPreviewBitmap.Clear(Color.FromArgb(0x00, 0x00, 0x00, 0x00));
                this.brushPreviewBitmap.Blit(pre_rect, brush_preview, brt_rect, WriteableBitmapExtensions.BlendMode.None);

                this.BrushPreviewImage.Source = this.brushPreviewBitmap;
            }
        }

        private void BrushRadiusSlider_ManipulationCompleted(object sender, ManipulationCompletedEventArgs e)
        {
            IsolatedStorageSettings.ApplicationSettings["BrushRadius"] = this.brushRadius;
            IsolatedStorageSettings.ApplicationSettings.Save();
        }

        private void BrushOpacitySlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            this.brushOpacity = e.NewValue;

            this.brushTemplateBitmap = new WriteableBitmap(this.brushRadius * 2, this.brushRadius * 2);

            for (int x = 0; x < this.brushTemplateBitmap.PixelWidth; x++)
            {
                for (int y = 0; y < this.brushTemplateBitmap.PixelHeight; y++)
                {
                    double r = Math.Sqrt(Math.Pow(x - this.brushRadius, 2) + Math.Pow(y - this.brushRadius, 2));

                    if (r <= this.brushRadius)
                    {
                        if (r <= this.brushRadius * this.brushOpacity)
                        {
                            this.brushTemplateBitmap.SetPixel(x, y, (0xFF << 24) | 0xFFFFFF);
                        }
                        else
                        {
                            this.brushTemplateBitmap.SetPixel(x, y, ((byte)(0xFF * (this.brushRadius - r) / (this.brushRadius * (1.0 - this.brushOpacity))) << 24) | 0xFFFFFF);
                        }
                    }
                    else
                    {
                        this.brushTemplateBitmap.SetPixel(x, y, (0x00 << 24) | 0xFFFFFF);
                    }
                }
            }

            if (this.editedBitmap != null)
            {
                int brush_width = Math.Max(1, Math.Min(Math.Min((int)(this.brushRadius / this.currentScale) * 2, this.editedBitmap.PixelWidth), this.editedBitmap.PixelHeight));

                this.brushBitmap = this.brushTemplateBitmap.Resize(brush_width, brush_width, WriteableBitmapExtensions.Interpolation.NearestNeighbor);
            }

            if (this.BrushRadiusSlider != null && this.BrushPreviewImage != null)
            {
                WriteableBitmap brush_preview = new WriteableBitmap(this.brushTemplateBitmap.PixelWidth,     this.brushTemplateBitmap.PixelHeight);
                this.brushPreviewBitmap       = new WriteableBitmap((int)this.BrushRadiusSlider.Maximum * 2, (int)this.BrushRadiusSlider.Maximum * 2);

                Rect brt_rect = new Rect(0, 0, this.brushTemplateBitmap.PixelWidth, this.brushTemplateBitmap.PixelHeight);
                Rect pre_rect = new Rect(((int)this.brushPreviewBitmap.PixelWidth  - this.brushTemplateBitmap.PixelWidth)  / 2,
                                         ((int)this.brushPreviewBitmap.PixelHeight - this.brushTemplateBitmap.PixelHeight) / 2,
                                         this.brushTemplateBitmap.PixelWidth,
                                         this.brushTemplateBitmap.PixelHeight);

                brush_preview.Clear(Colors.Red);
                brush_preview.Blit(brt_rect, this.brushTemplateBitmap, brt_rect, WriteableBitmapExtensions.BlendMode.Mask);

                this.brushPreviewBitmap.Clear(Color.FromArgb(0x00, 0x00, 0x00, 0x00));
                this.brushPreviewBitmap.Blit(pre_rect, brush_preview, brt_rect, WriteableBitmapExtensions.BlendMode.None);

                this.BrushPreviewImage.Source = this.brushPreviewBitmap;
            }
        }

        private void BrushOpacitySlider_ManipulationCompleted(object sender, ManipulationCompletedEventArgs e)
        {
            IsolatedStorageSettings.ApplicationSettings["BrushOpacity"] = this.brushOpacity;
            IsolatedStorageSettings.ApplicationSettings.Save();
        }
    }
}