using System;
using System.Collections.Generic;
using System.ComponentModel;
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
    public partial class SketchPage : PhoneApplicationPage
    {
        private const int MODE_NONE     = 0,
                          MODE_SCROLL   = 1,
                          MODE_ORIGINAL = 2,
                          MODE_EFFECTED = 3;

        private const int MAX_IMAGE_WIDTH  = 1400,
                          MAX_IMAGE_HEIGHT = 1400;

        private const int HELPER_POINT_WIDTH  = 6,
                          HELPER_POINT_HEIGHT = 6;

        private const int BRUSH_RADIUS = 24,
                          UNDO_DEPTH   = 4;

        private const double REDUCTION_MPIX_LIMIT = 1.0;

        class SketchGenTaskData
        {
            public int   radius;
            public int   width, height;
            public int[] pixels;
        }

        private bool                  loadImageOnLayoutUpdate,
                                      needImageReduction,
                                      editedImageChanged;
        private int                   selectedMode,
                                      gaussianRadius;
        private double                currentScale,
                                      initialScale;
        private List<int[]>           undoStack;
        private WriteableBitmap       loadedBitmap,
                                      editedBitmap,
                                      originalBitmap,
                                      effectedBitmap,
                                      helperBitmap,
                                      brushTemplateBitmap,
                                      brushBitmap;
        private BackgroundWorker      sketchGeneratorWorker;
        private MarketplaceDetailTask marketplaceDetailTask;

        public SketchPage()
        {
            InitializeComponent();

            this.loadImageOnLayoutUpdate = true;
            this.editedImageChanged      = false;
            this.selectedMode            = MODE_NONE;
            this.gaussianRadius          = 0;
            this.currentScale            = 1.0;
            this.initialScale            = 1.0;
            this.undoStack               = new List<int[]>();
            this.loadedBitmap            = null;
            this.editedBitmap            = null;
            this.originalBitmap          = null;
            this.effectedBitmap          = null;
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

            this.sketchGeneratorWorker                     = new BackgroundWorker();
            this.sketchGeneratorWorker.DoWork             += new DoWorkEventHandler(sketchGeneratorWorker_DoWork);
            this.sketchGeneratorWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler(sketchGeneratorWorker_RunWorkerCompleted);

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

            IDictionary<string, string> query_strings = this.NavigationContext.QueryString;

            if (query_strings.ContainsKey("radius"))
            {
                try
                {
                    this.gaussianRadius = Convert.ToInt32(query_strings["radius"]);
                }
                catch (Exception)
                {
                    // Ignore
                }
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
                this.loadedBitmap = bitmap;

                SketchGenTaskData task_data = new SketchGenTaskData();

                task_data.radius = this.gaussianRadius;
                task_data.width  = bitmap.PixelWidth;
                task_data.height = bitmap.PixelHeight;
                task_data.pixels = bitmap.Pixels;

                this.sketchGeneratorWorker.RunWorkerAsync(task_data);

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
            int   radius  = (int)(BRUSH_RADIUS / this.currentScale);
            Point t_point = new Point(touch_point.X / this.currentScale, touch_point.Y / this.currentScale);

            if (this.selectedMode == MODE_ORIGINAL || this.selectedMode == MODE_EFFECTED)
            {
                int width  = this.brushBitmap.PixelWidth;
                int height = this.brushBitmap.PixelHeight;

                Rect rect = new Rect();

                rect.X      = (rect.X = t_point.X - width  / 2 < 0 ? 0 : t_point.X - width  / 2) > this.editedBitmap.PixelWidth  - width  ? this.editedBitmap.PixelWidth  - width  : rect.X;
                rect.Y      = (rect.Y = t_point.Y - height / 2 < 0 ? 0 : t_point.Y - height / 2) > this.editedBitmap.PixelHeight - height ? this.editedBitmap.PixelHeight - height : rect.Y;
                rect.Width  = width;
                rect.Height = height;

                Rect brh = new Rect(0, 0, width, height);

                WriteableBitmap brush_bitmap = this.brushBitmap.Clone();

                if (this.selectedMode == MODE_ORIGINAL)
                {
                    brush_bitmap.Blit(brh, this.originalBitmap, rect, WriteableBitmapExtensions.BlendMode.Multiply);
                }
                else
                {
                    brush_bitmap.Blit(brh, this.effectedBitmap, rect, WriteableBitmapExtensions.BlendMode.Multiply);
                }

                this.editedBitmap.Blit(rect, brush_bitmap, brh, WriteableBitmapExtensions.BlendMode.Alpha);

                this.EditorImage.Source = this.editedBitmap;
            }
        }

        private void SketchPage_LayoutUpdated(object sender, EventArgs e)
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
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(AppResources.MessageBoxMessageImageOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                }

                this.loadImageOnLayoutUpdate = false;
            }
        }

        private void SketchPage_BackKeyPress(object sender, System.ComponentModel.CancelEventArgs e)
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

        private void sketchGeneratorWorker_DoWork(object sender, DoWorkEventArgs e)
        {
            int   radius = (e.Argument as SketchGenTaskData).radius;
            int   width  = (e.Argument as SketchGenTaskData).width;
            int   height = (e.Argument as SketchGenTaskData).height;
            int[] pixels = (e.Argument as SketchGenTaskData).pixels;
            
            // Make Gaussian blur of original image

            int[] sketch_pixels = pixels.Clone() as int[];

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
                    rgba[i] = ((sketch_pixels[s] >> (i * 8)) & 0xFF) << 4;
                }

                s += width;

                for (int j = r1; j < r2; j++, s += width)
                {
                    int p = 0;

                    for (int i = 0; i < 4; i++)
                    {
                        p = p | ((((rgba[i] += ((((sketch_pixels[s] >> (i * 8)) & 0xFF) << 4) - rgba[i]) * alpha / 16) >> 4) & 0xFF) << (i * 8));
                    }

                    sketch_pixels[s] = p;
                }
            }

            for (int row = r1; row <= r2; row++)
            {
                int s = row * width + c1;

                for (int i = 0; i < 4; i++)
                {
                    rgba[i] = ((sketch_pixels[s] >> (i * 8)) & 0xFF) << 4;
                }

                s++;

                for (int j = c1; j < c2; j++, s++)
                {
                    int p = 0;

                    for (int i = 0; i < 4; i++)
                    {
                        p = p | ((((rgba[i] += ((((sketch_pixels[s] >> (i * 8)) & 0xFF) << 4) - rgba[i]) * alpha / 16) >> 4) & 0xFF) << (i * 8));
                    }

                    sketch_pixels[s] = p;
                }
            }

            for (int col = c1; col <= c2; col++)
            {
                int s = r2 * width + col;

                for (int i = 0; i < 4; i++)
                {
                    rgba[i] = ((sketch_pixels[s] >> (i * 8)) & 0xFF) << 4;
                }

                s -= width;

                for (int j = r1; j < r2; j++, s -= width)
                {
                    int p = 0;

                    for (int i = 0; i < 4; i++)
                    {
                        p = p | ((((rgba[i] += ((((sketch_pixels[s] >> (i * 8)) & 0xFF) << 4) - rgba[i]) * alpha / 16) >> 4) & 0xFF) << (i * 8));
                    }

                    sketch_pixels[s] = p;
                }
            }

            for (int row = r1; row <= r2; row++)
            {
                int s = row * width + c2;

                for (int i = 0; i < 4; i++)
                {
                    rgba[i] = ((sketch_pixels[s] >> (i * 8)) & 0xFF) << 4;
                }

                s--;

                for (int j = c1; j < c2; j++, s--)
                {
                    int p = 0;

                    for (int i = 0; i < 4; i++)
                    {
                        p = p | ((((rgba[i] += ((((sketch_pixels[s] >> (i * 8)) & 0xFF) << 4) - rgba[i]) * alpha / 16) >> 4) & 0xFF) << (i * 8));
                    }

                    sketch_pixels[s] = p;
                }
            }

            // Make grayscale image from original image, inverted grayscale image from blurred image
            // Then apply Color Dodge mixing for sketch

            int[] grayscale_pixels = pixels.Clone() as int[];

            for (int i = 0; i < sketch_pixels.Length; i++)
            {
                byte[] b_color = BitConverter.GetBytes(sketch_pixels[i]);
                byte[] g_color = BitConverter.GetBytes(grayscale_pixels[i]);

                byte top_gray = (byte)(255 - (byte)((b_color[0] * 11 + b_color[1] * 16 + b_color[2] * 5) / 32));
                byte btm_gray =              (byte)((g_color[0] * 11 + g_color[1] * 16 + g_color[2] * 5) / 32);

                byte res_gray = (byte)(top_gray >= 255 ? 255 : Math.Min(btm_gray * 255 / (255 - top_gray), 255));

                sketch_pixels[i] = (b_color[3] << 24) | (res_gray << 16) | (res_gray << 8) | res_gray;
            }

            SketchGenTaskData task_data = new SketchGenTaskData();

            task_data.radius = radius;
            task_data.width  = width;
            task_data.height = height;
            task_data.pixels = sketch_pixels;

            e.Result = task_data;
        }

        private void sketchGeneratorWorker_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            this.GenerationProgressIndicator.IsVisible = false;

            if (!e.Cancelled && e.Error == null)
            {
                WriteableBitmap bitmap = new WriteableBitmap((e.Result as SketchGenTaskData).width, (e.Result as SketchGenTaskData).height);

                (e.Result as SketchGenTaskData).pixels.CopyTo(bitmap.Pixels, 0);

                this.editedImageChanged = true;
                this.selectedMode       = MODE_SCROLL;

                this.undoStack.Clear();

                this.originalBitmap = this.loadedBitmap.Clone();
                this.effectedBitmap = bitmap.Clone();
                this.editedBitmap   = bitmap.Clone();

                this.loadedBitmap = null;

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