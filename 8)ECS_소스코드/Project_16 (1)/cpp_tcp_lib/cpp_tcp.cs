using System.Diagnostics;
using System.IO;
using System.Net.Sockets;
using System.Text;

namespace cpp_tcp_lib
{
    public class cpp_Write_Read
    {
        public static TcpClient cpp_client;
        public static NetworkStream cpp_stream;

        public cpp_Write_Read()
        {
            cpp_client = new TcpClient("10.10.21.118", 5001);//10.10.21.118 <-- 이게 민건이 개발원 pc꺼, 192.168.35.240 
            cpp_stream = cpp_client.GetStream();
        }
        public void w_msg(string send_msg)
        {
            byte[] data = new byte[256];
            data = Encoding.UTF8.GetBytes(send_msg);
            cpp_stream.Write(data, 0, data.Length);
        }
        public void iw_msg(byte[] imageBytes)
        {
            string size = Convert.ToString(imageBytes.Length);
            Debug.WriteLine($"size ={size}");
            byte[] data = new byte[256];
            data = Encoding.UTF8.GetBytes(size);
            cpp_stream.Write(data,0, data.Length);
            Thread.Sleep(1000);
            cpp_stream.Write(imageBytes, 0, imageBytes.Length);
        }
        public string r_msg()
        {
            byte[] data1 = new byte[1024];
            int bytes = cpp_stream.Read(data1, 0, data1.Length);
            string responses = Encoding.UTF8.GetString(data1, 0, bytes);
            return responses;
        }
        public string rb_msg()
        {
            byte[] data1 = new byte[1];
            int bytes = cpp_stream.Read(data1, 0, data1.Length);
            string responses = data1[0].ToString();
            return responses;
        }
        public void close_cpp()
        {
            cpp_stream.Close();
            cpp_client.Close();
        }
    }
}