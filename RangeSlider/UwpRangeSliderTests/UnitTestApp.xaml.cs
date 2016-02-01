namespace UwpRangeSliderTests
{
    using Windows.ApplicationModel.Activation;
    using Windows.UI.Xaml;
    using Windows.UI.Xaml.Controls;

    using Microsoft.VisualStudio.TestPlatform.TestExecutor;

    sealed partial class App
    {
        public App()
        {
            InitializeComponent();
        }

        protected override void OnLaunched(LaunchActivatedEventArgs e)
        {
            Frame rootFrame = Window.Current.Content as Frame;
            if (rootFrame == null)
            {
                rootFrame = new Frame();
                Window.Current.Content = rootFrame;
            }
            
            UnitTestClient.CreateDefaultUI();
            Window.Current.Activate();
            UnitTestClient.Run(e.Arguments);
        }
    }
}
