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

namespace Project_16
{
    /// <summary>
    /// account.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class account : Page
    {
        public account()
        {
            InitializeComponent();
        }

        private void btn_cam_Click(object sender, RoutedEventArgs e)
        {
            NavigationService.Navigate(new Uri("/Camera.xaml", UriKind.Relative));
        }

        private void btn_log_Click(object sender, RoutedEventArgs e)
        {
            NavigationService.Navigate(new Uri("/LogCheck.xaml", UriKind.Relative));
        }
    }
}
