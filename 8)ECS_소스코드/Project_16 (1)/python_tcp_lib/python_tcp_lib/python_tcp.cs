using System.Diagnostics;
using System.Net.Sockets;
using System.Net;
using System.Text;

namespace python_tcp_lib
{
    public class python_Write_Read
    {
        public static TcpListener python_listener;
        public static NetworkStream python_stream;
        const string IP = "10.10.21.108"; // 파이썬 실행 컴퓨터 본체 혹은 와이파이 사용 시 연결 안됨 //192.168.0.153 <-- 민건 // 
        const int Port = 9877;

        public python_Write_Read()
        {
            python_listener = new TcpListener(IPAddress.Parse(IP), Port);
            // 서버 시작
            python_listener.Start();
            // 클라이언트 연결 대기
            TcpClient client = python_listener.AcceptTcpClient(); //대기하다가 성공시 블로킹 풀림
            // 클라이언트와 데이터 송수신을 위한 스트림 설정
            python_stream = client.GetStream();
        }
        public void w_msg(string send_msg)
        {
            byte[] data = new byte[256];
            data = Encoding.UTF8.GetBytes(send_msg);
            python_stream.Write(data, 0, data.Length);
        }
        public string r_msg()
        {
            while (true)
            {
                byte[] data = new byte[1024];
                int bytes = python_stream.Read(data, 0, data.Length);
                string result = Encoding.UTF8.GetString(data, 0, bytes);
                Debug.WriteLine(result);
                if (bytes > 0)
                {
                    return result;
                }
                else
                {
                    
                }
            }
        }
        public void close_python()
        {
            python_stream.Close();
            python_listener.Stop();
        }
    }
}
