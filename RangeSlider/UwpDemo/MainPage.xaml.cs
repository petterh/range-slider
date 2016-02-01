namespace UwpDemo
{
    using Windows.UI.Xaml;
    using Windows.UI.Xaml.Media;

    using UwpRangeSlider;

    public sealed partial class MainPage
    {
        public MainPage()
        {
            InitializeComponent();
        }

        private void OnCloseClicked(object sender, RoutedEventArgs e)
        {
            Application.Current.Exit();
        }

        private void OnToggleClicked(object sender, RoutedEventArgs e)
        {
            int count = VisualTreeHelper.GetChildrenCount(grid);
            for (int i = 0; i < count; i++)
            {
                RangeSlider rangeSlider = VisualTreeHelper.GetChild(grid, i) as RangeSlider;
                if (rangeSlider != null)
                {
                    rangeSlider.IsEnabled = !rangeSlider.IsEnabled;
                }
            }
        }
    }
}
