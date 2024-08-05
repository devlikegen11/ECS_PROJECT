using LiveCharts.Wpf;
using LiveCharts;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using LiveCharts.Defaults;
using Newtonsoft.Json.Linq;
using System.Diagnostics;
using LiveCharts.Definitions.Charts;

namespace Project_16
{
    public partial class LogCheck : Page
    {
        public static int red { get; set; }
        public static int green { get; set; }
        public static int blue { get; set; }
        public static int yellow { get; set; }
        public static int sum { get; set; }
        public LineSeries Good_product { get; set; }
        public LineSeries bad_product { get; set; }
        public SeriesCollection SeriesCollection1 { get; set; }
        public SeriesCollection SeriesCollection2 { get; set; }
        public ChartValues<ObservableValue> Values { get; set; }

        public LogCheck()
        {
            InitializeComponent();
            string page_protocol = "two";
            MainWindow.cpp_con.w_msg(page_protocol);
            Deserial();
            ShowChart();
            ShowBar();
            print_result();
        }

        public void ShowChart()
        {
            SeriesCollection1 = new SeriesCollection
            {
                new PieSeries
                {
                    Title = "파란색",
                    Values = new ChartValues<ObservableValue> { new ObservableValue(blue) },
                    DataLabels = true
                },
                new PieSeries
                {
                    Title = "빨간색",
                    Values = new ChartValues<ObservableValue> { new ObservableValue(red) },
                    DataLabels = true
                },
                new PieSeries
                {
                    Title = "노란색",
                    Values = new ChartValues<ObservableValue> { new ObservableValue(yellow) },
                    DataLabels = true
                },
                new PieSeries
                {
                    Title = "초록색",
                    Values = new ChartValues<ObservableValue> { new ObservableValue(green) },
                    DataLabels = true
                },
            };

            DataContext = this;
        }

        public void ShowBar()
        {
            var goodProductSeries = new ColumnSeries
            {
                Title = "Good Product",
                Values = new ChartValues<double> { sum - red }
            };

            var badProductSeries = new ColumnSeries
            {
                Title = "Bad Product",
                Values = new ChartValues<double> { red }
            };

            MyBarChart.Series = new SeriesCollection
            {
                goodProductSeries,
                badProductSeries
            };
        }

        private void Deserial()
        {
            string text = MainWindow.cpp_con.r_msg();
            Debug.WriteLine(text);
            JObject response = JObject.Parse(text);
            LogData(response["LOGS"]);
        }

        private void LogData(JToken column)
        {
            Values = new ChartValues<ObservableValue>();
            string MACHINE;
            string DETECTED;
            string COLOR;

            if (column != null)
            {
                foreach (var item in column)
                {
                    int count;
                    MACHINE = item["MACHINE"].ToString();
                    DETECTED = item["DETECTED"].ToString();
                    COLOR = item["COLOR"].ToString();
                    count = (int)item["COUNT"];

                    if (COLOR == "빨간색")
                    {
                        red += count;
                    }
                    else if (COLOR == "초록색")
                    {
                        green += count;
                    }
                    else if (COLOR == "파란색")
                    {
                        blue += count;
                    }
                    else if (COLOR == "노란색")
                    {
                        yellow += count;
                    }
                    Values.Add(new ObservableValue(count));
                    sum += count;
                }
            }
        }

        private void print_result()
        {
            resultBox.Text = "빨간색 : " + red + " 개\n" +
                             "노란색 : " + yellow + "개\n" +
                             "초록색 : " + green + "개\n" +
                             "파란색 : " + blue + "개";
        }

        private void btn_now_Click(object sender, RoutedEventArgs e)
        {
            NavigationService.Navigate(new Uri("/Camera.xaml", UriKind.Relative));
        }
    }
}
