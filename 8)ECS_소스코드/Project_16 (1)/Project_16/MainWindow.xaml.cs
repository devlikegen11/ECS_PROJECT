using System.Text;
using cpp_tcp_lib;
using python_tcp_lib;
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
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        static public cpp_Write_Read cpp_con = new cpp_Write_Read();
        //static public python_Write_Read python_con = new python_Write_Read();
        public MainWindow()
        {
            InitializeComponent();
        }
    }
}