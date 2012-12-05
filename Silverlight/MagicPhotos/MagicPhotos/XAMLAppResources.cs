using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace MagicPhotos
{
    public class XAMLAppResources
    {
        public XAMLAppResources()
        {
        }

        private static MagicPhotos.AppResources appResources = new MagicPhotos.AppResources();

        public MagicPhotos.AppResources AppResources { get { return appResources; } }
    }
}
