using OpenCvSharp;
using OpenCvSharp.WpfExtensions;
using System.Diagnostics;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
//using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.Windows.Xps;
using Project_16.cam;
using System.Drawing.Imaging;
using python_tcp_lib;



namespace Project_16
{
    /// <summary>
    /// InitPage.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class Camera : System.Windows.Controls.Page
    {
        Mat frame;
        Mat roiFrame;
        BitmapSource bitmapSource = null;
        python_Write_Read python_con;
        public Camera()
        {
            InitializeComponent();
            string page_protocol = "one";
            MainWindow.cpp_con.w_msg(page_protocol);
        }

        private void Page_Loaded()
        {
            using (VideoCapture capture = new VideoCapture(0))
            {
                Dispatcher.Invoke(() =>
                {
                    capture.FrameHeight = (int)cam1.Height;
                    capture.FrameWidth = (int)cam1.Width;
                });


                if (!capture.IsOpened())
                {
                    Console.WriteLine("웹캠을 열 수 없습니다.");
                    return;
                }

                // 창 생성
                //string windowName = "웹캠 출력";
                //Cv2.NamedWindow(windowName, WindowMode.AutoSize);

                frame = new Mat();
                Task.Run(() => wait_python_msg());
                while (true)
                {
                    // 프레임 읽기
                    capture.Read(frame);

                    if (frame.Empty())
                    {
                        Console.WriteLine("프레임을 읽을 수 없습니다.");
                        break;
                    }

                    // 관심 영역(ROI) 설정 (예: 화면의 중앙 영역)
                    OpenCvSharp.Rect roi = new OpenCvSharp.Rect(100, 100, 200, 100); // (x, y, width, height)
                    roiFrame = new Mat(frame, roi);

                    // ROI를 표시

                    bitmapSource = BitmapSourceConverter.ToBitmapSource(roiFrame);
                    Dispatcher.Invoke(() =>
                    {
                        cam1.Source = bitmapSource;
                    });

                    // 'q' 키를 누르면 종료
                    if (Cv2.WaitKey(30) == 'q')
                    {
                        break;
                    }
                    
                }

                // 자원 해제
                //Cv2.DestroyAllWindows();
            }
        }
        private void wait_python_msg()
        {
            int count = 0;
            while (true)
            {
                Debug.WriteLine($"count = {count}");
                count++;
                string capture_protocol = python_con.r_msg(); //여기서 블로킹이 걸려야 하지않나?
                Debug.WriteLine($"capture_protocol = {capture_protocol}");
                if (capture_protocol == "99") //파이썬이 보내는 사진찍으라는 신호
                {
                    Dispatcher.Invoke(() =>
                    {
                        capture_img.Source = bitmapSource; //현재 frame 이미지 바인딩
                    });
                    MainWindow.cpp_con.w_msg("AA");
                    byte[] bytes = roiFrame.ToBytes(".png");
                    MainWindow.cpp_con.iw_msg(bytes);       //cpp에 이미지 전송
                    Task.Delay(5000);

                    string result_msg = MainWindow.cpp_con.rb_msg(); //cpp에게 검출결과를 받아옴.
                    Debug.WriteLine($"result_msg = {result_msg}");
                    Dispatcher.Invoke(() =>                         // 받아온 결과를 바탕으로 텍스트 박스에 출력
                    {
                        if (result_msg == "0")
                        {
                            python_con.w_msg("20");
                            txb_result_image.Text = "red 불량";
                        }
                        if (result_msg == "1")
                        {
                            python_con.w_msg("10");
                            txb_result_image.Text = "green";
                        }
                        else if (result_msg == "2")
                        {
                            python_con.w_msg("10");
                            txb_result_image.Text = "blue";
                        }
                        else if (result_msg == "3")
                        {
                            python_con.w_msg("10");
                            txb_result_image.Text = "yellow";
                        }

                        else if (result_msg == "10")
                        {
                            python_con.w_msg("30");
                            txb_result_image.Text = "보류";
                        }
                    });
                }
            }
        }

        private void btn_log_Click(object sender, RoutedEventArgs e)
        {
            NavigationService.Navigate(new Uri("/LogCheck.xaml", UriKind.Relative));
        }

        private void load_canvas_Click(object sender, RoutedEventArgs e)
        {
            python_con = new python_Write_Read();
            Page_Loaded();
        }

    }
}