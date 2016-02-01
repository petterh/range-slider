namespace UwpRangeSliderTests
{
    using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
    using Microsoft.VisualStudio.TestPlatform.UnitTestFramework.AppContainer;

    using UwpRangeSlider;

    using Assert = Microsoft.VisualStudio.TestPlatform.UnitTestFramework.Assert;

    [TestClass]
    public class RangeSliderTest
    {
        [UITestMethod]
        public void TestDefaultInitialization()
        {
            RangeSlider rangeSlider = new RangeSlider();

            Assert.AreEqual(0d, rangeSlider.Lower);
            Assert.AreEqual(100d, rangeSlider.Upper);
        }

        [UITestMethod]
        public void TestLower()
        {
            RangeSlider rangeSlider = new RangeSlider();

            Assert.AreEqual(0d, rangeSlider.Lower);
            rangeSlider.SetValue(RangeSlider.LowerProperty, 50d);
            Assert.AreEqual(50d, rangeSlider.Lower);
            Assert.AreEqual(50d, rangeSlider.GetValue(RangeSlider.LowerProperty));
        }

        [UITestMethod]
        public void TestUpper()
        {
            RangeSlider rangeSlider = new RangeSlider();

            Assert.AreEqual(100d, rangeSlider.Upper);
            rangeSlider.SetValue(RangeSlider.UpperProperty, 50d);
            Assert.AreEqual(50d, rangeSlider.Upper);
            Assert.AreEqual(50d, rangeSlider.GetValue(RangeSlider.UpperProperty));
        }
    }
}
