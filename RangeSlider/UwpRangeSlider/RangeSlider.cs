namespace UwpRangeSlider
{
    using System.Diagnostics;

    using Windows.UI.Xaml;
    using Windows.UI.Xaml.Controls;
    using Windows.UI.Xaml.Controls.Primitives;

    /// <summary>
    /// Slider template: https://msdn.microsoft.com/en-us/library/windows/apps/mt299153.aspx
    /// Thumb template: https://msdn.microsoft.com/en-us/library/windows/apps/mt299155.aspx
    /// </summary>
    public class RangeSlider : Slider
    {
        public static readonly DependencyProperty LowerProperty = DependencyProperty.Register(
            nameof(Lower),
            typeof(double),
            typeof(RangeSlider),
            new PropertyMetadata(0, OnLowerChanged));

        public static readonly DependencyProperty UpperProperty = DependencyProperty.Register(
            nameof(Upper),
            typeof(double),
            typeof(RangeSlider),
            new PropertyMetadata(100d, OnUpperChanged));

        private Thumb leftThumb;

        public double Lower
        {
            get { return (double)GetValue(LowerProperty); }
            set { SetValue(LowerProperty, value); }
        }

        public double Upper
        {
            get { return (double)GetValue(UpperProperty); }
            set { SetValue(UpperProperty, value); }
        }

        protected override void OnApplyTemplate()
        {
            base.OnApplyTemplate();
            leftThumb = GetTemplateChild<Thumb>("LeftThumb");
            leftThumb.CanDrag = true;
            leftThumb.DragStarted += (sender, args) => Debug.WriteLine($"Drag started: {args.HorizontalOffset}");
            leftThumb.DragDelta += (sender, args) => Debug.WriteLine($"Drag delta: {args.HorizontalChange}");
        }

        private static void OnLowerChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        private static void OnUpperChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
        }

        private T GetTemplateChild<T>(string name)
            where T : DependencyObject
        {
            return GetTemplateChild(name) as T;
        }
    }
}
