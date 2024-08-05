#include <iostream>
#include <mariadb/conncpp.hpp>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <curl/curl.h>
#include <fstream>
#include "json.hpp"
#include "opencv2/opencv.hpp"

#define BUF_SIZE 1024
#define MAX_CLNT 256
#define uchar unsigned char
#define protnum 5001

using json = nlohmann::json; // nlohmann::json를 json으로 간소화
using namespace cv;
using namespace std;

void *backgroundthread(void *arg);
void rev_store_image(int clnt_sock);
void error_handling(const char *msg)
{
    std::cerr << msg << '\n';
    exit(1);
}

int clnt_cnt = 0;         // 소켓 디스크립터의 인덱스 번호 역할
int clnt_socks[MAX_CLNT]; // 소켓 디스크립터 담을 256개
pthread_mutex_t mutx; // 뮤텍스 변수

enum protocol
{
    red = 0,
    green = 1,
    blue = 2,
    yellow = 3,
    postpone = 10
};

/////////////////////////////////////////////////////////////////////////////
class DB
{
public:
    sql::Connection *conn;
    DB() {}
    void connect()
    {
        try
        {
            sql::Driver *driver = sql::mariadb::get_driver_instance();
            sql::SQLString url = ("jdbc:mariadb://127.0.0.1:3306/DETECT");
            sql::Properties properties({{"user", "ALL"}, {"password", "1234"}});
            conn = driver->connect(url, properties);
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "Error Connecting to MariaDB Platform: " << e.what() << std::endl;
        }
    }

    ~DB() { delete conn; }
};
/////////////////////////////////////////////////////////////////////////////
void remove_clnt_serv(int clnt_sock, DB &database)
{
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++)
    {
        if (clnt_sock == clnt_socks[i])
        {
            while (i++ < clnt_cnt - 1)
                clnt_socks[i] = clnt_socks[i + 1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
}
std::string read_msg(int clnt_sock, DB &data)
{
    std::string str_msg;
    char msg[BUF_SIZE];
    memset(msg, 0, sizeof(msg));
    if (read(clnt_sock, msg, sizeof(msg)) > 0)
    {
        str_msg = msg;
        return str_msg;
    }
    else
    {
        remove_clnt_serv(clnt_sock, data);
        return "연결종료";
    }
}
/////////////////////////////////////////////////////////////////////////////
class Task_image
{

    DB &Task_image_DB;
    int clnt_sock;
    uchar protocol[5] = {red, green, blue, yellow, postpone};
    std::string machine_name;
    std::string image_path;
    Mat fist_image;

public:
    Task_image(DB &database, int clnt_sock) : Task_image_DB(database), clnt_sock(clnt_sock) {}

    void handling_image()
    {
        while (1)
        {
            std::cout << "페이지 선택" << std::endl;
            std::string page_choice = read_msg(clnt_sock, Task_image_DB);
            std::cout << page_choice << std::endl;
            if(page_choice =="연결종료")
            {
                break;
            }
            else if (page_choice == "one")
            {
                machine_name = read_msg(clnt_sock, Task_image_DB);
                if (machine_name != "연결종료")
                {
                    receive_image();      // 기계이름 및 이미지 수신
                    open_image();         // 이미지 열기
                    pretreatment_image(); // 전처리
                }
                else
                {
                    break;
                }
            }

            else if (page_choice == "two")
            {
                std::string log_str = data_send().dump();
                std::cout<<"log_str = "<<log_str<<std::endl;
                write(clnt_sock, log_str.c_str(), log_str.length());
                // 디비 정보 전송
            }
        }
    }

    void receive_image()
    {
        char buffer[BUF_SIZE];
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::string now_str = std::ctime(&now_c);

        now_str.erase(std::remove(now_str.begin(), now_str.end(), '\n'), now_str.end());

        image_path = "/home/uhvuk01/공개/LMS/PJ/image";
        image_path += now_str + ".jpg";
        std::ofstream outfile(image_path, std::ios::out | std::ios::binary);
        std::cout << "바이너리 파일 준비완료\n";
        int bytes = 0;
        int Read_count = 0;
        int receive_image_str = 0;

        std::string image_size = read_msg(clnt_sock, Task_image_DB);
        int image_size_int = std::stoi(image_size);
        std::cout << image_size << std::endl;

        memset(buffer, 0, sizeof(buffer));
        while (receive_image_str < image_size_int && (bytes = read(clnt_sock, buffer, BUF_SIZE)) > 0)
        {
            Read_count++;
            std::cout << "Read_count = " << Read_count << std::endl;
            receive_image_str += bytes;
            outfile.write(buffer, bytes);
        }
        memset(buffer, 0, sizeof(buffer));
        outfile.close();
    }

    void open_image()
    {
        std::cout << "open이미지 함수 진입\n";
        // 이미지 파일 로드
        fist_image = imread(image_path);

        // 이미지가 제대로 로드되었는지 확인
        if (fist_image.empty())
        {
            std::cerr << "Error: Could not read the image file '" << image_path << "'" << std::endl;
        }
    }

    void pretreatment_image()
    {
        std::cout << "pretreatment_image 함수 진입\n";

        Mat hsv_image;
        Mat green_image, blue_image, yellow_image, red1_image, red2_image;

        // imshow("Original Image", fist_image);
        // waitKey(0);
        // destroyAllWindows();
        cvtColor(fist_image, hsv_image, COLOR_BGR2HSV);

        inRange(hsv_image, Scalar(35, 100, 100), Scalar(85, 255, 255), green_image);
        inRange(hsv_image, Scalar(100, 100, 100), Scalar(140, 255, 255), blue_image);
        inRange(hsv_image, Scalar(20, 100, 100), Scalar(30, 255, 255), yellow_image);
        inRange(hsv_image, Scalar(0, 100, 100), Scalar(10, 255, 255), red1_image);
        inRange(hsv_image, Scalar(160, 100, 100), Scalar(179, 255, 255), red2_image);

        // 컨투어 찾기
        std::vector<std::vector<Point>> contours_image;

        if (contours_fill_check(clnt_sock, green_image, contours_image) || contours_fill_check(clnt_sock, blue_image, contours_image) || contours_fill_check(clnt_sock, yellow_image, contours_image) || contours_fill_check(clnt_sock, red1_image, contours_image) || contours_fill_check(clnt_sock, red2_image, contours_image))
        {
            /* 보류 메시지 보냈으니 실행할 로직 없음 */
        }
        else
        {
            // contours_color_check(clnt_sock, green_image, contours_image, green);
            // contours_color_check(clnt_sock, blue_image, contours_image, blue);
            // contours_color_check(clnt_sock, yellow_image, contours_image, yellow);
            // contours_color_check(clnt_sock, red1_image, contours_image, red);
            // contours_color_check(clnt_sock, red2_image, contours_image, red);

            if (contours_color_check(clnt_sock, green_image, contours_image, green))
            {
            }
            else if (contours_color_check(clnt_sock, blue_image, contours_image, blue))
            {
            }
            else if (contours_color_check(clnt_sock, yellow_image, contours_image, yellow))
            {
            }
            else if (contours_color_check(clnt_sock, red1_image, contours_image, red))
            {
            }
            else if (contours_color_check(clnt_sock, red2_image, contours_image, red))
            {
            }
        }
    }

    bool contours_fill_check(int clnt_sock, cv::Mat HSV_image, std::vector<std::vector<Point>> contours)
    {
        std::cout << "contours_fill_check 함수 진입\n";
        findContours(HSV_image, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        bool is_detect = false;
        // 컨투어 내부가 완전히 채워졌는지 확인
        for (size_t i = 0; i < contours.size(); i++)
        {
            // 컨투어를 기준으로 빈 마스크 생성
            Mat contour_mask = Mat::zeros(HSV_image.size(), CV_8UC1);

            // 컨투어를 채움
            drawContours(contour_mask, contours, static_cast<int>(i), Scalar(255), FILLED);

            // 원래 마스크의 해당 컨투어 영역만 비교
            Mat original_mask = HSV_image.clone();
            drawContours(original_mask, contours, static_cast<int>(i), Scalar(0), FILLED);

            int filled_area = countNonZero(contour_mask);
            int original_area = countNonZero(HSV_image) - countNonZero(original_mask);

            // 만약 채워진 영역과 원래 마스크의 영역이 다르다면, 내부가 완전히 채워지지 않은 것
            if (filled_area != original_area)
            {
                drawContours(HSV_image, contours, static_cast<int>(i), Scalar(0, 0, 255), 2);
                std::cout << "완전히 채워지지 않은 컨투어 발견" << std::endl;
                is_detect = true;
                break;
            }
        }

        if (is_detect == true)
        {
            std::cout << " 보류상태 확인\n";
            write(clnt_sock, &protocol[4], sizeof(protocol[4])); // protocol[4] = postpone = 10
            try
            {
                std::unique_ptr<sql::PreparedStatement> pre_stmnt(Task_image_DB.conn->prepareStatement("INSERT INTO RESULT (MACHINE_NAME, DETECTED_PRODUCT, COLOR) VALUES ('AA', ?, ?)"));
                pre_stmnt->setInt(1, 0);
                pre_stmnt->setString(2, "빨간색");
                std::unique_ptr<sql::ResultSet> res(pre_stmnt->executeQuery());
            }
            catch (sql::SQLException &e)
            {
                std::cerr << "Error Connecting to MariaDB Platform: " << e.what() << std::endl;
            }
            return true;
        }
        else
        {
            std::cout << "불량/양품 상태 구분 가능해짐\n";
            return false;
        }
    }

    bool contours_color_check(int clnt_sock, cv::Mat HSV_image, std::vector<std::vector<Point>> contours, int color)
    {
        std::cout << "contours_color_check 함수 진입\n";
        findContours(HSV_image, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        if (color == green && !contours.empty()) // 녹색에 대한 흰색이 있다면
        {
            write(clnt_sock, &protocol[1], sizeof(protocol[1])); // green = 1
            std::cout << "초록색 검출\n";

            try
            {
                std::unique_ptr<sql::PreparedStatement> pre_stmnt(Task_image_DB.conn->prepareStatement("INSERT INTO RESULT (MACHINE_NAME, DETECTED_PRODUCT, COLOR) VALUES ('AA', ?, ?)"));
                pre_stmnt->setInt(1, 1);
                pre_stmnt->setString(2, "초록색");
                std::unique_ptr<sql::ResultSet> res(pre_stmnt->executeQuery());
                return true;
            }
            catch (sql::SQLException &e)
            {
                std::cerr << "Error Connecting to MariaDB Platform: " << e.what() << std::endl;
            }
        }

        else if (color == blue && !contours.empty()) // 파란색에 대한 흰색이 있다면
        {
            write(clnt_sock, &protocol[2], sizeof(protocol[2])); // blue = 2
            std::cout << "파란색 검출\n";
            try
            {
                std::unique_ptr<sql::PreparedStatement> pre_stmnt(Task_image_DB.conn->prepareStatement("INSERT INTO RESULT (MACHINE_NAME, DETECTED_PRODUCT, COLOR) VALUES ('AA', ?, ?)"));
                pre_stmnt->setInt(1, 1);
                pre_stmnt->setString(2, "파란색");
                std::unique_ptr<sql::ResultSet> res(pre_stmnt->executeQuery());
                return true;
            }
            catch (sql::SQLException &e)
            {
                std::cerr << "Error Connecting to MariaDB Platform: " << e.what() << std::endl;
            }
        }

        else if (color == yellow && !contours.empty()) // 노란색에 대한 흰색이 있다면
        {
            write(clnt_sock, &protocol[3], sizeof(protocol[3])); // yellow = 3
            std::cout << "노란색 검출\n";
            try
            {
                std::unique_ptr<sql::PreparedStatement> pre_stmnt(Task_image_DB.conn->prepareStatement("INSERT INTO RESULT (MACHINE_NAME, DETECTED_PRODUCT, COLOR) VALUES ('AA', ?, ?)"));
                pre_stmnt->setInt(1, 1);
                pre_stmnt->setString(2, "노란색");
                std::unique_ptr<sql::ResultSet> res(pre_stmnt->executeQuery());
                return true;
            }
            catch (sql::SQLException &e)
            {
                std::cerr << "Error Connecting to MariaDB Platform: " << e.what() << std::endl;
            }
        }

        else if (color == red && !contours.empty()) // 빨간색에 대한 흰색이 있다면
        {
            write(clnt_sock, &protocol[0], sizeof(protocol[0])); // red = 0
            std::cout << "빨간색 검출\n";
            try
            {
                std::unique_ptr<sql::PreparedStatement> pre_stmnt(Task_image_DB.conn->prepareStatement("INSERT INTO RESULT (MACHINE_NAME, DETECTED_PRODUCT, COLOR) VALUES ('AA', ?, ?)"));
                pre_stmnt->setInt(1, 0);
                pre_stmnt->setString(2, "빨간색");
                std::unique_ptr<sql::ResultSet> res(pre_stmnt->executeQuery());
                return true;
            }
            catch (sql::SQLException &e)
            {
                std::cerr << "Error Connecting to MariaDB Platform: " << e.what() << std::endl;
            }
        }

        return false;
        // else // 컨투어링 시 완벽한 마스킹은 되었으나 등록되어있는 색상이 아닐 때
        // {
        //     std::cout<<"다른색 검출\n";
        //     write(clnt_sock,&protocol[4],sizeof(protocol[4])); //protocol[4] = postpone = 10
        //     try {
        //         std::unique_ptr<sql::PreparedStatement> pre_stmnt(Task_image_DB.conn->prepareStatement("INSERT INTO RESULT VALUES (DEFAULT, ?, ?, NULL, DEFAULT)"));
        //         pre_stmnt->setString(1,machine_name);
        //         pre_stmnt->setInt(2,2);
        //         std::unique_ptr<sql::ResultSet> res(pre_stmnt->executeQuery());
        //     }
        //     catch(sql::SQLException& e){
        //         std::cerr << "Error Connecting to MariaDB Platform: " << e.what() << std::endl;
        //     }
        // }
    }

    json data_send()
    {
        json send_data;
        std::vector<json> data_logs;

        try
        {
            std::unique_ptr<sql::PreparedStatement> pre_stmnt(Task_image_DB.conn->prepareStatement("SELECT MACHINE_NAME, DETECTED_PRODUCT, COLOR, COUNT(*) AS count FROM RESULT GROUP BY COLOR ORDER BY count DESC;"));
            std::unique_ptr<sql::ResultSet> res1(pre_stmnt->executeQuery());
            while (res1->next())
            {
                json grapic_data;
                grapic_data["MACHINE"] = res1->getString("MACHINE_NAME");
                grapic_data["DETECTED"] = res1->getString("DETECTED_PRODUCT");
                grapic_data["COLOR"] = res1->getString("COLOR");
                grapic_data["COUNT"] = res1->getInt("count");
                data_logs.emplace_back(grapic_data);
            }
            send_data["LOGS"] = data_logs;
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
        return send_data;
    }
};

/////////////////////////////////////////////////////////////////////////////

int main()
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    pthread_t t_id;

    pthread_mutex_init(&mutx, NULL);
    serv_sock = socket(PF_INET, SOCK_STREAM, 0); // 소킷 파일 디스크립터(서버)

    // if (argc != 2)
    // {
    //     printf("Usage : %s <port>\n", argv[0]);
    //     exit(1);
    // }
    
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(protnum); // atoi(argv[1])


    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");


    while (1)
    {
        try
        {
            clnt_adr_sz = sizeof(clnt_adr);
            clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz); //클라이언트의 소킷 파일 디스크립터 (int 4,5,6,7,8)
            printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));

            pthread_mutex_lock(&mutx);
            clnt_socks[clnt_cnt++] = clnt_sock;
            pthread_mutex_unlock(&mutx);

            pthread_create(&t_id, NULL, backgroundthread, (void *)&clnt_sock);
            pthread_detach(t_id);
        }
        catch (std::exception &e)
        {
            std::cerr << "error : " << &e << std::endl;
        }
    }
    close(serv_sock);
    return 0;
}

void *backgroundthread(void *arg)
{
    int clnt_sock = *((int *)arg);
    DB database;
    database.connect();
    Task_image Ti_instance(database, clnt_sock);
    Ti_instance.handling_image();
    clnt_cnt ++;

    return 0;
}
