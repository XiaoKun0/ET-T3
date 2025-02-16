/*
 * Copyright (C) 2025  F_Error11
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __ERROR_TOOLS_T3_HPP
#define __ERROR_TOOLS_T3_HPP
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <netdb.h>
#include <arpa/inet.h>
#include <termios.h>

#include "tools/json.hpp"
#include "tools/md5.hpp"

using namespace std;

// The Full Name Of This NameSpace Is ErrorTools
namespace ET {
    string StrToHex(const string& input) {
        stringstream ss;
        ss << hex << uppercase;
        for (unsigned char c : input)
            ss << setw(2) << setfill('0') << static_cast<int>(c);
        return ss.str();
    }

    string TimeStamp() {  // 秒钟级时间戳
        auto now = chrono::system_clock::now();
        chrono::seconds epoch_seconds = chrono::duration_cast<chrono::seconds>(
            now.time_since_epoch()
        );
        return to_string(epoch_seconds.count());
    }

    string TimeDot() {  // 分钟级时间点
        auto now = chrono::system_clock::now();
        time_t now_time_t = chrono::system_clock::to_time_t(now);
        tm* tm_info = localtime(&now_time_t);
        ostringstream oss;
        oss << put_time(tm_info, "%Y%m%d%H%M");
        return oss.str();
    }

    string RC4(const string& key, const string& data) {
        vector<unsigned char> s(256);
        int j = 0;

        for (int i = 0; i < 256; ++i) s[i] = i;

        for (int i = 0; i < 256; ++i) {
            unsigned char key_byte = 0;
            if (!key.empty())
                key_byte = static_cast<unsigned char>(key[i % key.size()]);
        
            j = (j + s[i] + key_byte) % 256;
            swap(s[i], s[j]);
        }

        string output;
        int i = 0;
        j = 0;
    
        for (size_t k = 0; k < data.size(); ++k) {
            i = (i + 1) % 256;
            j = (j + s[i]) % 256;
            swap(s[i], s[j]);
            unsigned char k_byte = s[(s[i] + s[j]) % 256];
            output += data[k] ^ k_byte;
        }

        return output;
    }

    // 这个函数是我24年7月不知道在哪扣下来的
    // 因为https请求还是要依赖大型三方库，所有还是用这个
    // 或者也可以换成shell-curl指令，但我不希望外部调用
    string SedHttpReq(const string &url) {
        string host;
        string path = "/";
        int port = 80;

        size_t scheme_end = url.find("://");
        if (scheme_end != string::npos) {
            size_t host_start = scheme_end + 3;
            size_t host_end = url.find('/', host_start);
            if (host_end == string::npos) host_end = url.length();
            host = url.substr(host_start, host_end - host_start);
            size_t port_start = host.find(':');
            if (port_start != string::npos) {
                port = stoi(host.substr(port_start + 1));
                host = host.substr(0, port_start);
            }
            path = url.substr(host_end);
        } else {
            host = url;
        }

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("socket");
            return "";
        }

        hostent *server = gethostbyname(host.c_str());
        if (server == nullptr) {
            herror("gethostbyname");
            close(sock);
            return "";
        }

        sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        serv_addr.sin_port = htons(port);

        if (connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("connect");
            close(sock);
            return "";
        }

        ostringstream request_stream;
        request_stream << "GET " << path << " HTTP/1.1\r\n";
        request_stream << "Host: " << host << "\r\n";
        request_stream << "Connection: close\r\n\r\n";
        string request = request_stream.str();

        if (send(sock, request.c_str(), request.length(), 0) < 0) {
            perror("send");
            close(sock);
            return "";
        }

        string response;
        char buffer[1024];
        int bytes_received;
        while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0)
            response.append(buffer, bytes_received);
        
        close(sock);
    
        size_t pos = response.find("\r\n\r\n");
        if (pos != string::npos) {
            string headers = response.substr(0, pos);
            string body = response.substr(pos + 4);
            return body;  // 返回响应体 (这里才是正常返回)
        }
    
        return "";
    }

    // 不格式化响应内容会很乱，无法被解密，所有有了格式化函数
    string FormatRes(const string& input) {
        stringstream ss(input);
        string line;
        vector<string> lines;

        while (getline(ss, line)) lines.push_back(line);

        if (lines.size() <= 3) return "";
    
        // 跳过第一行和最后两行(这就是主要的垃圾内容)
        vector<string> middleLines(lines.begin() + 1, lines.end() - 2);

        string result;
        for (const auto& l : middleLines) result += l + "\n";

        if (!result.empty() && result.back() == '\n') result.pop_back();

        return result;
    }

    // 如果是加密的响应，格式化后还有异常符号，不切一下会导致解析json失败
    string CutJSON(const string& str) {
        size_t pos = str.find_last_of('}');
        if (pos != string::npos) {
            return str.substr(0, pos + 1);
        }
        return "";
    }

    string DeviceID() {  // 获取设备码(依赖shell)
        string imei;
        FILE* pipe = popen("getprop ro.serialno", "r");
        if (!pipe) {
            cerr << "无法打开管道" << endl;
            return imei;
        }

        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            imei += buffer;
        }
        pclose(pipe);
        imei.erase(remove(imei.begin(), imei.end(), '\n'), imei.end());
        return imei;
    }
    std::string PrivateInput() {
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        std::string input;
        while (1) {
            char ch;
            ssize_t n = read(STDIN_FILENO, &ch, 1);
            if (n == -1) break;
            else if (n == 0) continue;
            else {
                if (ch == '\n' || ch == '\r') break;
                input += ch;
                cout << "*" << flush;
            }
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        cout << endl;
        return input;
    }
    
/* - - - - - - - - - - - - [ 以 上 为 辅 助 函 数 ] - - - - - - - - - - - - */
    inline static string NoticeUrl = "";        // 公告接口
    inline static string LoginUrl = "";         // 登录接口
    inline static string GetVerUrl = "";        // 获取版本号接口
    inline static string HeartBeatUrl = "";     // 心跳验证接口
    inline static int HeartBeatinterval = 10;   // 心跳验证间隔时间
    inline static string APPKEY = "";         // APPKEY
    inline static string RC4KEY = "";         // RC4密钥
    inline static bool 返回值加密 = false;     // 返回值加密
    inline static bool 请求值加密 = false;     // 请求值加密
    inline static bool 卡密隐私输入 = false;   // 卡密隐私输入
    /*   ! ! ! 禁 止 直 接 修 改 以 下 变 量 ! ! !   */
    string key;                               // 卡密(此处设为全局变量，供心跳验证使用)
    string LoginStatusCode;                  // 登录状态码(心跳验证使用)
    atomic<bool> HeartbeatFlag(false);       // 心跳验证循环控制
    int counter = 0;                          // 心跳解析错误导致失败的计数器
    
    bool NOTICE()
    {
        string 时间戳 = TimeStamp();
        string 参数, 签名, URL;
        if (请求值加密)
        {
            参数 = "t=" + StrToHex(RC4(RC4KEY, 时间戳));
            签名 = GetMD5(参数 + "&" + APPKEY);
            URL = NoticeUrl + "&" + 参数 + "&s=" + StrToHex(RC4(RC4KEY, 签名));
        } else {
            参数 = "t=" + 时间戳;
            签名 = GetMD5(参数 + "&" + APPKEY);
            URL = NoticeUrl + "&" + 参数 + "&s=" + 签名;
        }
        
        string SerRes;
        if (返回值加密)  // 请求服务器
        {
            SerRes = CutJSON(RC4(RC4KEY, FormatRes(SedHttpReq(URL))));
        } else {
            SerRes = CutJSON(FormatRes(SedHttpReq(URL)));
        }
        
        if (SerRes.empty()) {
            cout << "Server not responding! " << endl;
            return false;
        }

        nlohmann::json json_obj;
        try {
            json_obj = nlohmann::json::parse(SerRes);
        } catch (nlohmann::json::parse_error& e) {
            cerr << "解析响应错误: " << e.what() << endl;
            return false;
        }
        
        int code = json_obj["code"];
        string res = json_obj["msg"];
        cout << res << endl;
        if (code == 200) return true;
        return false;
    }
    
    bool LOGIN()
    {
        cout << "请输入卡密: " << flush;
        if (卡密隐私输入) 
        {
            key = PrivateInput();
        } else {
            getline(cin, key);
        }
        if (key == "")
        {
            cout << "Key is empty! " << endl;
            return false;
        }
    
        string 设备ID = DeviceID();
        string 时间戳 = TimeStamp();
        string 参数, 签名, URL;
        if (请求值加密)
        {
            参数 = "kami=" + StrToHex(RC4(RC4KEY, key)) + "&imei=" + StrToHex(RC4(RC4KEY, 设备ID)) + "&t=" + StrToHex(RC4(RC4KEY, 时间戳));
            签名 = GetMD5(参数 + "&" + APPKEY);
            URL = LoginUrl + "&" + 参数 + "&s=" + StrToHex(RC4(RC4KEY, 签名));
        } else {
            参数 = "kami=" + key + "&imei=" + 设备ID + "&t=" + 时间戳;
            签名 = GetMD5(参数 + "&" + APPKEY);
            URL = LoginUrl + "&" + 参数 + "&s=" + 签名;
        }
    
        string SerRes;
        if (返回值加密)  // 请求服务器
        {
            SerRes = CutJSON(RC4(RC4KEY, FormatRes(SedHttpReq(URL))));
        } else {
            SerRes = CutJSON(FormatRes(SedHttpReq(URL)));
        }
    
        if (SerRes.empty()) {
            cout << "Server not responding! " << endl;
            return false;
        }

        nlohmann::json json_obj;
        try {
            json_obj = nlohmann::json::parse(SerRes);
        } catch (nlohmann::json::parse_error& e) {
            cerr << "解析响应错误: " << e.what() << endl;
            return false;
        }
        
        int code = json_obj["code"];
        if (code == 200)  // 登陆成功
        {
            string ID = json_obj["id"];
            string EndTime = json_obj["end_time"];
            string Token = json_obj["token"];
            
            LoginStatusCode = json_obj["statecode"];
            
            string LocalToken = GetMD5(ID + APPKEY + 参数 + "&" + APPKEY + EndTime +  TimeDot());

            if (LocalToken != Token)
            {
                cout << "签名状态异常" << endl;
                return false;
            } else {
                cout << "到期时间: " << EndTime << endl;
                return true;
            }
        } else {
            string res = json_obj["msg"];
            cout << res << endl;
            return false;
        }
    }
    
    int GetVersion()
    {
        string 时间戳 = TimeStamp();
        string 参数, 签名, URL;
        if (请求值加密)
        {
            参数 = "t=" + StrToHex(RC4(RC4KEY, 时间戳));
            签名 = GetMD5(参数 + "&" + APPKEY);
            URL = GetVerUrl + "&" + 参数 + "&s=" + StrToHex(RC4(RC4KEY, 签名));
        } else {
            参数 = "t=" + 时间戳;
            签名 = GetMD5(参数 + "&" + APPKEY);
            URL = GetVerUrl + "&" + 参数 + "&s=" + 签名;
        }
        
        string SerRes;
        if (返回值加密)  // 请求服务器
        {
            SerRes = CutJSON(RC4(RC4KEY, FormatRes(SedHttpReq(URL))));
        } else {
            SerRes = CutJSON(FormatRes(SedHttpReq(URL)));
        }
        
        if (SerRes.empty()) {
            cout << "Server not responding! " << endl;
            return -1;
        }

        nlohmann::json json_obj;
        try {
            json_obj = nlohmann::json::parse(SerRes);
        } catch (nlohmann::json::parse_error& e) {
            cerr << "解析响应错误: " << e.what() << endl;
            return -1;
        }
        
        int code = json_obj["code"];
        string res = json_obj["msg"];
        if (code != 200) 
        {
            cout << res << endl;
            return -1;
        }
        return stoi(res);
    }
    
    int BeatOnce(const string& BeatUrl)
    {
        string SerRes;
        if (返回值加密)  // 请求服务器
        {
            SerRes = CutJSON(RC4(RC4KEY, FormatRes(SedHttpReq(BeatUrl))));
        } else {
            SerRes = CutJSON(FormatRes(SedHttpReq(BeatUrl)));
        }
    
        if (SerRes.empty()) {
            cout << "Server not responding! " << endl;
            return -1;
        }

        nlohmann::json json_obj;
        try {
            json_obj = nlohmann::json::parse(SerRes);
        } catch (nlohmann::json::parse_error& e) {
            cerr << "解析响应错误: " << e.what() << endl;
            return 1;
        }
    
        int code = json_obj["code"];
        if (code != 200) return -1;
        
        return 0;
    }
    
    void HeartBeatThread(const string& URL, atomic<bool>& stopFlag) {
        while (stopFlag.load()) {
            int result = BeatOnce(URL);
            {
                switch (result)
                {
                    case 1:
                        counter++;
                        if (counter >= 3) exit(1);
                        break;
                    case -1:
                        exit(1);
                        break;
                    case 0:
                        cout << "心跳验证成功" << endl;
                        break;
                }
            }
            this_thread::sleep_for(chrono::seconds(HeartBeatinterval));
        }
    }
    
    bool HeartBeat(bool Switch)
    {
        if (HeartbeatFlag.load() == Switch)
        {
            if (Switch)
            {
                cout << "当前已开启心跳验证，无法再次开启" << endl;
            } else {
                cout << "当前未开启心跳验证，无法执行关闭" << endl;
            }
            return false;
        }
        string URL;
        if (Switch) {
            string 时间戳 = TimeStamp();
            string 参数, 签名;
            if (请求值加密)
            {
                参数 = "kami=" + StrToHex(RC4(RC4KEY, key)) + "&statecode=" + StrToHex(RC4(RC4KEY, LoginStatusCode)) + "&t=" + StrToHex(RC4(RC4KEY, 时间戳));
                签名 = GetMD5(参数 + "&" + APPKEY);
                URL = HeartBeatUrl + "&" + 参数 + "&s=" + StrToHex(RC4(RC4KEY, 签名));
            } else {
                参数 = "kami=" + key + "&statecode=" + LoginStatusCode + "&t=" + 时间戳;
                签名 = GetMD5(参数 + "&" + APPKEY);
                URL = HeartBeatUrl + "&" + 参数 + "&s=" + 签名;
            }
            
            HeartbeatFlag.store(true);  // 设置子线程循环条件
            thread heartbeatThread(HeartBeatThread, URL, ref(HeartbeatFlag));
            heartbeatThread.detach(); // 将子线程放到后台运行
            return true;
        } else {
            HeartbeatFlag.store(false);  // 停止子线程循环
            cout << "心跳验证被终止" << endl;
            return true;
        }
    }
}
#endif  // __ERROR_TOOLS_T3_HPP