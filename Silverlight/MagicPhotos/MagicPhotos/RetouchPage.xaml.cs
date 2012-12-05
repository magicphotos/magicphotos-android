using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.IO;
using System.IO.IsolatedStorage;
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
    public partial class RetouchPage : PhoneApplicationPage
    {
        private const int MODE_NONE           = 0,
                          MODE_SCROLL         = 1,
                          MODE_SAMPLING_POINT = 2,
                          MODE_CLONE          = 3,
                          MODE_BLUR           = 4;

        private const int MAX_IMAGE_WIDTH  = 1400,
                          MAX_IMAGE_HEIGHT = 1400;

        private const int HELPER_POINT_WIDTH  = 6,
                          HELPER_POINT_HEIGHT = 6;

        private const int BRUSH_RADIUS = 24,
                          UNDO_DEPTH   = 4;

        private const double REDUCTION_MPIX_LIMIT = 1.0;

        private static int[,] GAUSSIAN_KERNEL = {{1,  4,  7,  4,  1},
                                                 {4,  16, 26, 16, 4},
                                                 {7,  26, 41, 26, 7},
                                                 {4,  16, 26, 16, 4},
                                                 {1,  4,  7,  4,  1}};

        private bool                  loadImageOnLayoutUpdate,
                                      loadImageCancelled,
                                      needImageReduction,
                                      editedImageChanged,
                                      samplingPointValid;
        private int                   selectedMode;
        private double                currentScale,
                                      initialScale;
        private List<int[]>           undoStack;
        private Point                 samplingPoint,
                                      initialSamplingPoint,
                                      clonePoint,
                                      initialClonePoint,
                                      blurPoint;
        private WriteableBitmap       editedBitmap,
                                      helperBitmap,
                                      brushTemplateBitmap,
                                      brushBitmap;
        private PhotoChooserTask      photoChooserTask;
        private MarketplaceDetailTask marketplaceDetailTask;

        public RetouchPage()
        {
            InitializeComponent();

            this.loadImageOnLayoutUpdate = true;
            this.loadImageCancelled      = false;
            this.editedImageChanged      = false;
            this.samplingPointValid      = false;
            this.selectedMode            = MODE_NONE;
            this.currentScale            = 1.0;
            this.initialScale            = 1.0;
            this.undoStack               = new List<int[]>();
            this.samplingPoint           = new Point(0, 0);
            this.initialSamplingPoint    = new Point(0, 0);
            this.clonePoint              = new Point(0, 0);
            this.initialClonePoint       = new Point(0, 0);
            this.blurPoint               = new Point(0, 0);
            this.editedBitmap            = null;
            this.helperBitmap            = null;
            this.brushBitmap             = null;

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

            this.brushTemplateBitmap = new WriteableBitmap(BRUSH_RADIUS * 2, BRUSH_RADIUS * 2);

            for (int x = 0; x < this.brushTemplateBitmap.PixelWidth; x++)
            {
                for (int y = 0; y < this.brushTemplateBitmap.PixelHeight; y++)
                {
                    if (Math.Sqrt(Math.Pow(x - BRUSH_RADIUS, 2) + Math.Pow(y - BRUSH_RADIUS, 2)) <= BRUSH_RADIUS)
                    {
                        this.brushTemplateBitmap.SetPixel(x, y, (0xFF << 24) | 0xFFFFFF);
                    }
                    else
                    {
                        this.brushTemplateBitmap.SetPixel(x, y, (0x00 << 24) | 0xFFFFFF);
                    }
                }
            }

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

            if (this.selectedMode == MODE_SAMPLING_POINT)
            {
                this.SamplingPointModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_sampling_point_selected.png", UriKind.Relative)) };
            }
            else
            {
                this.SamplingPointModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_sampling_point.png", UriKind.Relative)) };
            }

            if (this.selectedMode == MODE_CLONE)
            {
                this.CloneModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_clone_selected.png", UriKind.Relative)) };
            }
            else
            {
                this.CloneModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_clone.png", UriKind.Relative)) };
            }

            if (this.selectedMode == MODE_BLUR)
            {
                this.BlurModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_blur_selected.png", UriKind.Relative)) };
            }
            else
            {
                this.BlurModeButton.Background = new ImageBrush { ImageSource = new BitmapImage(new Uri(img_dir + "/mode_blur.png", UriKind.Relative)) };
            }
        }

        private void UpdateSamplingPointImage()
        {
            if (this.samplingPointValid)
            {
                this.SamplingPointImage.Margin     = new Thickness(this.samplingPoint.X - (this.SamplingPointImage.ActualWidth / 2), this.samplingPoint.Y - (this.SamplingPointImage.ActualHeight / 2), 0, 0);
                this.SamplingPointImage.Visibility = System.Windows.Visibility.Visible;
            }
            else
            {
                this.SamplingPointImage.Visibility = System.Windows.Visibility.Collapsed;
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
                int width  = (int)(this.HelperImage.Width  / this.currentScale) < this.editedBitmap.PixelWidth  ? (int)(this.HelperImage.Width  / this.currentScale) : this.editedBitmap.PixelWidth;
                int height = (int)(this.HelperImage.Height / this.currentScale) < this.editedBitmap.PixelHeight ? (int)(this.HelperImage.Height / this.currentScale) : this.editedBitmap.PixelHeight;

                int x = (x = (int)(touch_point.X / this.currentScale) - width  / 2 < 0 ? 0 : (int)(touch_point.X / this.currentScale) - width  / 2) > this.editedBitmap.PixelWidth  - width  ? this.editedBitmap.PixelWidth  - width  : x;
                int y = (y = (int)(touch_point.Y / this.currentScale) - height / 2 < 0 ? 0 : (int)(touch_point.Y / this.currentScale) - height / 2) > this.editedBitmap.PixelHeight - height ? this.editedBitmap.PixelHeight - height : y;

                this.helperBitmap = this.editedBitmap.Crop(x, y, width, height);

                int touch_x1 = (int)(touch_point.X / this.currentScale) - x - HELPER_POINT_WIDTH  / 2;
                int touch_y1 = (int)(touch_point.Y / this.currentScale) - y - HELPER_POINT_HEIGHT / 2;
                int touch_x2 = (int)(touch_point.X / this.currentScale) - x + HELPER_POINT_WIDTH  / 2;
                int touch_y2 = (int)(touch_point.Y / this.currentScale) - y + HELPER_POINT_HEIGHT / 2;

                if (touch_x1 > 0 && touch_x1 < this.helperBitmap.PixelWidth && touch_y1 > 0 && touch_y1 < this.helperBitmap.PixelHeight &&
                    touch_x2 > 0 && touch_x2 < this.helperBitmap.PixelWidth && touch_y2 > 0 && touch_y2 < this.helperBitmap.PixelHeight)
                {
                    this.helperBitmap.DrawRectangle(touch_x1, touch_y1, touch_x2, touch_y2, 0x00FFFFFF);
                }

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
            if (this.needImageReduction && bitmap.PixelWidth * bitmap.PixelHeight > REDUCTION_MPIX_LIMIT * 1000000.0)
            {
                double factor = Math.Sqrt((bitmap.PixelWidth * bitmap.PixelHeight) / (REDUCTION_MPIX_LIMIT * 1000000.0));

                bitmap = bitmap.Resize((int)(bitmap.PixelWidth / factor), (int)(bitmap.PixelHeight / factor), WriteableBitmapExtensions.Interpolation.NearestNeighbor);
            }

            if (bitmap.PixelWidth != 0 && bitmap.PixelHeight != 0)
            {
                this.editedImageChanged = false;
                this.samplingPointValid = false;
                this.selectedMode       = MODE_SCROLL;

                this.undoStack.Clear();

                this.editedBitmap = bitmap;

                if (this.editedBitmap.PixelWidth > this.editedBitmap.PixelHeight)
                {
                    this.currentScale = this.EditorScrollViewer.ViewportWidth / this.editedBitmap.PixelWidth;
                }
                else
                {
                    this.currentScale = this.EditorScrollViewer.ViewportHeight / this.editedBitmap.PixelHeight;
                }

                this.EditorImage.Visibility = System.Windows.Visibility.Visible;
                this.EditorImage.Source     = this.editedBitmap;
                this.EditorImage.Width      = this.editedBitmap.PixelWidth  * this.currentScale;
                this.EditorImage.Height     = this.editedBitmap.PixelHeight * this.currentScale;

                int brush_width  = (int)(BRUSH_RADIUS / this.currentScale) * 2 < this.editedBitmap.PixelWidth  ? (int)(BRUSH_RADIUS / this.currentScale) * 2 : this.editedBitmap.PixelWidth;
                int brush_height = (int)(BRUSH_RADIUS / this.currentScale) * 2 < this.editedBitmap.PixelHeight ? (int)(BRUSH_RADIUS / this.currentScale) * 2 : this.editedBitmap.PixelHeight;

                this.brushBitmap = this.brushTemplateBitmap.Resize(brush_width, brush_height, WriteableBitmapExtensions.Interpolation.NearestNeighbor);

                UpdateModeButtons();
                UpdateSamplingPointImage();
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

        private void ChangeBitmap()
        {
            int   radius         = (int)(BRUSH_RADIUS / this.currentScale);
            Point sampling_point = new Point(this.samplingPoint.X / this.currentScale, this.samplingPoint.Y / this.currentScale);
            Point clone_point    = new Point(this.clonePoint.X    / this.currentScale, this.clonePoint.Y    / this.currentScale);
            Point blur_point     = new Point(this.blurPoint.X     / this.currentScale, this.blurPoint.Y     / this.currentScale);

            if (this.selectedMode == MODE_CLONE)
            {
                if (this.samplingPointValid)
                {
                    int width  = this.brushBitmap.PixelWidth;
                    int height = this.brushBitmap.PixelHeight;

                    Rect src = new Rect();

                    src.X      = (src.X = sampling_point.X - width  / 2 < 0 ? 0 : sampling_point.X - width  / 2) > this.editedBitmap.PixelWidth  - width  ? this.editedBitmap.PixelWidth  - width  : src.X;
                    src.Y      = (src.Y = sampling_point.Y - height / 2 < 0 ? 0 : sampling_point.Y - height / 2) > this.editedBitmap.PixelHeight - height ? this.editedBitmap.PixelHeight - height : src.Y;
                    src.Width  = width;
                    src.Height = height;

                    Rect dst = new Rect();

                    dst.X      = (dst.X = clone_point.X - width  / 2 < 0 ? 0 : clone_point.X - width  / 2) > this.editedBitmap.PixelWidth  - width  ? this.editedBitmap.PixelWidth  - width  : dst.X;
                    dst.Y      = (dst.Y = clone_point.Y - height / 2 < 0 ? 0 : clone_point.Y - height / 2) > this.editedBitmap.PixelHeight - height ? this.editedBitmap.PixelHeight - height : dst.Y;
                    dst.Width  = width;
                    dst.Height = height;

                    Rect brh = new Rect(0, 0, width, height);

                    WriteableBitmap brush_bitmap = this.brushBitmap.Clone();

                    brush_bitmap.Blit(brh, this.editedBitmap, src, WriteableBitmapExtensions.BlendMode.Multiply);
                    this.editedBitmap.Blit(dst, brush_bitmap, brh, WriteableBitmapExtensions.BlendMode.Alpha);

                    this.EditorImage.Source = this.editedBitmap;
                }
            }
            else if (this.selectedMode == MODE_BLUR)
            {
                int width  = radius * 2 <= this.editedBitmap.PixelWidth  ? radius * 2 : this.editedBitmap.PixelWidth;
                int height = radius * 2 <= this.editedBitmap.PixelHeight ? radius * 2 : this.editedBitmap.PixelHeight;

                Rect blr = new Rect(0, 0, width, height);

                Rect src = new Rect();

                src.X      = (src.X = blur_point.X - width  / 2 < 0 ? 0 : blur_point.X - width  / 2) > this.editedBitmap.PixelWidth  - width  ? this.editedBitmap.PixelWidth  - width  : src.X;
                src.Y      = (src.Y = blur_point.Y - height / 2 < 0 ? 0 : blur_point.Y - height / 2) > this.editedBitmap.PixelHeight - height ? this.editedBitmap.PixelHeight - height : src.Y;
                src.Width  = width;
                src.Height = height;

                WriteableBitmap blur_bitmap = new WriteableBitmap(width, height);

                blur_bitmap.Blit(blr, this.editedBitmap, src, WriteableBitmapExtensions.BlendMode.None);

                blur_bitmap = blur_bitmap.Convolute(GAUSSIAN_KERNEL);

                this.editedBitmap.Blit(src, blur_bitmap, blr, WriteableBitmapExtensions.BlendMode.None);

                this.EditorImage.Source = this.editedBitmap;
            }
        }

        private void RetouchPage_LayoutUpdated(object sender, EventArgs e)
        {
            if (this.loadImageOnLayoutUpdate)
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
                    MessageBox.Show(AppResources.MessageBoxMessageImageOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                }

                this.loadImageOnLayoutUpdate = false;
            }
        }

        private void RetouchPage_BackKeyPress(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (this.editedImageChanged)
            {
                MessageBoxResult result = MessageBox.Show(AppResources.MessageBoxMessageUnsavedImageQuestion, AppResources.MessageBoxHeaderWarning, MessageBoxButton.OKCancel);

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

        private void SamplingPointModeButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.selectedMode != MODE_NONE)
            {
                this.selectedMode = MODE_SAMPLING_POINT;

                UpdateModeButtons();
            }
        }

        private void CloneModeButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.selectedMode != MODE_NONE)
            {
                this.selectedMode = MODE_CLONE;

                UpdateModeButtons();
            }
        }

        private void BlurModeButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.selectedMode != MODE_NONE)
            {
                this.selectedMode = MODE_BLUR;

                UpdateModeButtons();
            }
        }

        private void SaveButton_Click(object sender, EventArgs e)
        {
            if (this.editedBitmap != null)
            {
                if ((Application.Current as App).TrialMode)
                {
                    MessageBoxResult result = MessageBox.Show(AppResources.MessageBoxMessageTrialVersionQuestion, AppResources.MessageBoxHeaderInfo, MessageBoxButton.OKCancel);

                    if (result == MessageBoxResult.OK)
                    {
                        try
                        {
                            this.marketplaceDetailTask.Show();
                        }
                        catch (Exception ex)
                        {
                            MessageBox.Show(AppResources.MessageBoxMessageMarketplaceOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
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

                        MessageBox.Show(AppResources.MessageBoxMessageImageSavedInfo, AppResources.MessageBoxHeaderInfo, MessageBoxButton.OK);
                    }
                    catch (Exception ex)
                    {
                        MessageBox.Show(AppResources.MessageBoxMessageImageSaveError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                    }
                }
            }
        }

        private void HelpButton_Click(object sender, EventArgs e)
        {
            NavigationService.Navigate(new Uri("/HelpPage.xaml", UriKind.Relative));
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

            if (this.selectedMode == MODE_SAMPLING_POINT)
            {
                this.samplingPointValid = true;
                this.samplingPoint      = e.GetPosition(this.EditorImage);

                UpdateSamplingPointImage();

                UpdateHelper(true, e.GetPosition(this.EditorImage));
            }
            else if (this.selectedMode == MODE_CLONE)
            {
                if (this.samplingPointValid)
                {
                    this.editedImageChanged = true;

                    this.clonePoint = e.GetPosition(this.EditorImage);

                    this.initialSamplingPoint = this.samplingPoint;
                    this.initialClonePoint    = this.clonePoint;

                    SaveUndoImage();

                    ChangeBitmap();

                    UpdateHelper(true, e.GetPosition(this.EditorImage));
                }
            }
            else if (this.selectedMode == MODE_BLUR)
            {
                this.editedImageChanged = true;

                this.blurPoint = e.GetPosition(this.EditorImage);

                SaveUndoImage();

                ChangeBitmap();

                UpdateHelper(true, e.GetPosition(this.EditorImage));
            }
        }

        private void EditorImage_MouseMove(object sender, MouseEventArgs e)
        {
            if (this.selectedMode == MODE_SAMPLING_POINT)
            {
                this.samplingPointValid = true;
                this.samplingPoint      = e.GetPosition(this.EditorImage);

                UpdateSamplingPointImage();

                UpdateHelper(true, e.GetPosition(this.EditorImage));
            }
            else if (this.selectedMode == MODE_CLONE)
            {
                if (this.samplingPointValid)
                {
                    Point sampling_point = new Point(this.initialSamplingPoint.X + e.GetPosition(this.EditorImage).X - this.initialClonePoint.X,
                                                     this.initialSamplingPoint.Y + e.GetPosition(this.EditorImage).Y - this.initialClonePoint.Y);

                    if (sampling_point.X >= 0 && sampling_point.X < this.EditorImage.ActualWidth &&
                        sampling_point.Y >= 0 && sampling_point.Y < this.EditorImage.ActualHeight)
                    {
                        this.samplingPoint = sampling_point;
                        this.clonePoint    = e.GetPosition(this.EditorImage);

                        ChangeBitmap();

                        UpdateSamplingPointImage();
                    }

                    UpdateHelper(true, e.GetPosition(this.EditorImage));
                }
            }
            else if (this.selectedMode == MODE_BLUR)
            {
                this.blurPoint = e.GetPosition(this.EditorImage);

                ChangeBitmap();

                UpdateHelper(true, e.GetPosition(this.EditorImage));
            }
        }

        private void EditorImage_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            this.EditorImage.ReleaseMouseCapture();

            if (this.selectedMode == MODE_SAMPLING_POINT || this.selectedMode == MODE_CLONE || this.selectedMode == MODE_BLUR)
            {
                UpdateHelper(false, e.GetPosition(this.EditorImage));
            }
        }

        private void EditorImage_MouseLeave(object sender, MouseEventArgs e)
        {
            if (this.selectedMode == MODE_SAMPLING_POINT || this.selectedMode == MODE_CLONE || this.selectedMode == MODE_BLUR)
            {
                UpdateHelper(false, e.GetPosition(this.EditorImage));
            }
        }

        private void EditorImage_PinchStarted(object sender, PinchStartedGestureEventArgs e)
        {
            if (this.selectedMode == MODE_SCROLL)
            {
                e.Handled = true;

                this.initialScale = this.currentScale;

                this.samplingPointValid = false;

                UpdateSamplingPointImage();
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

                if ((width >= this.EditorScrollViewer.ViewportWidth || height >= this.EditorScrollViewer.ViewportHeight) &&
                    (width <= MAX_IMAGE_WIDTH                       && height <= MAX_IMAGE_HEIGHT))
                {
                    this.currentScale       = scale;
                    this.EditorImage.Width  = width;
                    this.EditorImage.Height = height;

                    int brush_width  = (int)(BRUSH_RADIUS / this.currentScale) * 2 < this.editedBitmap.PixelWidth  ? (int)(BRUSH_RADIUS / this.currentScale) * 2 : this.editedBitmap.PixelWidth;
                    int brush_height = (int)(BRUSH_RADIUS / this.currentScale) * 2 < this.editedBitmap.PixelHeight ? (int)(BRUSH_RADIUS / this.currentScale) * 2 : this.editedBitmap.PixelHeight;

                    this.brushBitmap = this.brushTemplateBitmap.Resize(brush_width, brush_height, WriteableBitmapExtensions.Interpolation.NearestNeighbor);
                }
            }
        }

        private void EditorScrollViewer_ManipulationStarted(object sender, ManipulationStartedEventArgs e)
        {
            if (this.selectedMode != MODE_SCROLL)
            {
                e.Handled = true;
                e.Complete();
            }
        }

        private void EditorScrollViewer_ManipulationDelta(object sender, ManipulationDeltaEventArgs e)
        {
            if (this.selectedMode != MODE_SCROLL)
            {
                e.Handled = true;
                e.Complete();
            }
        }

        private void EditorScrollViewer_ManipulationCompleted(object sender, ManipulationCompletedEventArgs e)
        {
            if (this.selectedMode != MODE_SCROLL)
            {
                e.Handled = true;
            }
        }
    }
}