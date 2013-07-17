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
using System.Windows.Shapes;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using Microsoft.Xna.Framework.Media;

namespace MagicPhotos
{
    public partial class MainPage : PhoneApplicationPage
    {
        private string startupImageToken;
        
        public MainPage()
        {
            InitializeComponent();

            this.startupImageToken = "";

            ApplicationBarIconButton button;

            button        = new ApplicationBarIconButton(new Uri("/Images/help.png", UriKind.Relative));
            button.Text   = AppResources.ApplicationBarButtonHelpText;
            button.Click += new EventHandler(HelpButton_Click);

            this.ApplicationBar.Buttons.Add(button);
        }

        protected override void OnNavigatedTo(System.Windows.Navigation.NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            if (this.startupImageToken == "")
            {
                IDictionary<string, string> query_strings = this.NavigationContext.QueryString;

                if (query_strings.ContainsKey("token"))
                {
                    this.startupImageToken = query_strings["token"];
                }
            }
        }

        private void createTemporaryImage(string token)
        {
            WriteableBitmap bitmap = new WriteableBitmap(0, 0);

            if (token != null && token != "")
            {
                using (MediaLibrary library = new MediaLibrary())
                {
                    Picture picture = library.GetPictureFromToken(token);

                    if (picture != null)
                    {
                        bitmap.SetSource(picture.GetImage());
                    }
                }
            }
            else
            {
                throw new Exception("Internal error");
            }

            using (IsolatedStorageFile store = IsolatedStorageFile.GetUserStoreForApplication())
            {
                string file_name = "image.jpg";

                if (store.FileExists(file_name))
                {
                    store.DeleteFile(file_name);
                }

                using (IsolatedStorageFileStream stream = store.CreateFile(file_name))
                {
                    bitmap.SaveJpeg(stream, bitmap.PixelWidth, bitmap.PixelHeight, 0, 100);
                }
            }
        }

        private void DecolorizeOpenImageButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.startupImageToken != "")
            {
                try
                {
                    createTemporaryImage(this.startupImageToken);

                    NavigationService.Navigate(new Uri("/DecolorizePage.xaml", UriKind.Relative));
                }
                catch (Exception ex)
                {
                    MessageBox.Show(AppResources.MessageBoxMessageImageOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                }
            }
            else
            {
                NavigationService.Navigate(new Uri("/DecolorizePage.xaml", UriKind.Relative));
            }
        }

        private void SketchOpenImageButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.startupImageToken != "")
            {
                try
                {
                    createTemporaryImage(this.startupImageToken);

                    NavigationService.Navigate(new Uri("/SketchPreviewPage.xaml", UriKind.Relative));
                }
                catch (Exception ex)
                {
                    MessageBox.Show(AppResources.MessageBoxMessageImageOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                }
            }
            else
            {
                NavigationService.Navigate(new Uri("/SketchPreviewPage.xaml", UriKind.Relative));
            }
        }

        private void CartoonOpenImageButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.startupImageToken != "")
            {
                try
                {
                    createTemporaryImage(this.startupImageToken);

                    NavigationService.Navigate(new Uri("/CartoonPreviewPage.xaml", UriKind.Relative));
                }
                catch (Exception ex)
                {
                    MessageBox.Show(AppResources.MessageBoxMessageImageOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                }
            }
            else
            {
                NavigationService.Navigate(new Uri("/CartoonPreviewPage.xaml", UriKind.Relative));
            }
        }

        private void BlurOpenImageButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.startupImageToken != "")
            {
                try
                {
                    createTemporaryImage(this.startupImageToken);

                    NavigationService.Navigate(new Uri("/BlurPreviewPage.xaml", UriKind.Relative));
                }
                catch (Exception ex)
                {
                    MessageBox.Show(AppResources.MessageBoxMessageImageOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                }
            }
            else
            {
                NavigationService.Navigate(new Uri("/BlurPreviewPage.xaml", UriKind.Relative));
            }
        }

        private void PixelateOpenImageButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.startupImageToken != "")
            {
                try
                {
                    createTemporaryImage(this.startupImageToken);

                    NavigationService.Navigate(new Uri("/PixelatePreviewPage.xaml", UriKind.Relative));
                }
                catch (Exception ex)
                {
                    MessageBox.Show(AppResources.MessageBoxMessageImageOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                }
            }
            else
            {
                NavigationService.Navigate(new Uri("/PixelatePreviewPage.xaml", UriKind.Relative));
            }
        }

        private void RecolorOpenImageButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.startupImageToken != "")
            {
                try
                {
                    createTemporaryImage(this.startupImageToken);

                    NavigationService.Navigate(new Uri("/RecolorPage.xaml", UriKind.Relative));
                }
                catch (Exception ex)
                {
                    MessageBox.Show(AppResources.MessageBoxMessageImageOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                }
            }
            else
            {
                NavigationService.Navigate(new Uri("/RecolorPage.xaml", UriKind.Relative));
            }
        }

        private void RetouchOpenImageButton_Click(object sender, RoutedEventArgs e)
        {
            if (this.startupImageToken != "")
            {
                try
                {
                    createTemporaryImage(this.startupImageToken);

                    NavigationService.Navigate(new Uri("/RetouchPage.xaml", UriKind.Relative));
                }
                catch (Exception ex)
                {
                    MessageBox.Show(AppResources.MessageBoxMessageImageOpenError + " " + ex.Message.ToString(), AppResources.MessageBoxHeaderError, MessageBoxButton.OK);
                }
            }
            else
            {
                NavigationService.Navigate(new Uri("/RetouchPage.xaml", UriKind.Relative));
            }
        }

        private void HelpButton_Click(object sender, EventArgs e)
        {
            NavigationService.Navigate(new Uri("/HelpPage.xaml", UriKind.Relative));
        }
    }
}