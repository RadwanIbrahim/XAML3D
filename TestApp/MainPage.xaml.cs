using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace TestApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
        }
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);

        }
        double y = 0;
        double x = 0;
        private void OnManipulationDelta(object sender, ManipulationDeltaRoutedEventArgs e)
        {
            y += e.Delta.Translation.Y;
            x += e.Delta.Translation.X;
            (sender as DirectXModel.DXModel).Transform.Rotation = new System.Numerics.Vector3((float)x, (float)y, 0.0f);
            ((sender as DirectXModel.DXModel).RenderTransform as TranslateTransform).X = x;
            ((sender as DirectXModel.DXModel).RenderTransform as TranslateTransform).Y= y;
        }

        private void OnBackClicked(object sender, RoutedEventArgs e)
        {
            Frame.GoBack();
        }

        private void OnModelLoaded(object sender, bool e)
        {

        }

       
    }
}
