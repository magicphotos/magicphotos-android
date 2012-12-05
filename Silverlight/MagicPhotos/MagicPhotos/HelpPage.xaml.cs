using System;
using System.Collections.Generic;
using System.Globalization;
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
using System.Windows.Resources;
using System.Windows.Shapes;
using Microsoft.Phone.Controls;

namespace MagicPhotos
{
    public partial class HelpPage : PhoneApplicationPage
    {
        private static string[] HELP_FILES = {"Help/help.html",
                                              "Help/help.de-DE.html",
                                              "Help/help.fr-FR.html",
                                              "Help/help.ru-RU.html",
                                              "Help/original_image.png", 
                                              "Help/sampling_point_selected.png",
                                              "Help/clone_in_progress.png",
                                              "Help/final_image.png",
                                              "Help/mode_scroll_button.png",
                                              "Help/mode_original_button.png",
                                              "Help/mode_effected_button.png",
                                              "Help/mode_color_button.png",
                                              "Help/mode_sampling_point_button.png",
                                              "Help/mode_clone_button.png",
                                              "Help/mode_blur_button.png"};

        public HelpPage()
        {
            InitializeComponent();

            try
            {
                using (IsolatedStorageFile store = IsolatedStorageFile.GetUserStoreForApplication())
                {
                    for (int i = 0; i < HELP_FILES.Length; i++)
                    {
                        string   full_path = string.Empty;
                        string   delim     = "/";
                        string[] path      = HELP_FILES[i].Split(delim.ToCharArray());

                        for (int j = 0; j < path.Length - 1; j++)
                        {
                            full_path = System.IO.Path.Combine(full_path, path[j]);

                            store.CreateDirectory(full_path);
                        }

                        if (store.FileExists(HELP_FILES[i]))
                        {
                            store.DeleteFile(HELP_FILES[i]);
                        }

                        StreamResourceInfo resource = Application.GetResourceStream(new Uri(HELP_FILES[i], UriKind.Relative));

                        using (BinaryReader reader = new BinaryReader(resource.Stream))
                        {
                            byte[] data = reader.ReadBytes((int)resource.Stream.Length);

                            using (BinaryWriter writer = new BinaryWriter(store.CreateFile(HELP_FILES[i])))
                            {
                                writer.Write(data);
                            }
                        }
                    }
                }
            }
            catch (Exception)
            {
                // Ignore
            }
        }

        protected override void OnNavigatedTo(System.Windows.Navigation.NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            string help_file         = "Help/help.html";
            string culture_help_file = String.Format("Help/help.{0}.html", CultureInfo.CurrentCulture.Name);

            try
            {
                using (IsolatedStorageFile store = IsolatedStorageFile.GetUserStoreForApplication())
                {
                    if (store.FileExists(culture_help_file))
                    {
                        help_file = culture_help_file;
                    }
                }
            }
            catch (Exception)
            {
                // Ignore
            }

            this.HelpWebBrowser.Navigate(new Uri(help_file, UriKind.Relative));
        }
    }
}