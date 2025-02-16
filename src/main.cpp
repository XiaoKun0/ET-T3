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

#include "et.hpp"  // 随手搓的终端工具(实际环境无序包含此头)
/* - - - - - 以 下 为 T 3 验 证 使 用 例 子 - - - - - */

#include <iostream>
#include <string>

#include "ErrTools/ET_T3.hpp"  // 包含此头文件以使用ET_T3

int main() {
    // T3需求变量 (全部函数)
    ET::APPKEY = "APPKEY";
    ET::RC4KEY = "RC4KEY";
    ET::返回值加密 = true;
    ET::请求值加密 = true;
    // 登录接口
    ET::LoginUrl = "http://w.t3yanzheng.com/XXXXXX";
    // 公告接口
    ET::NoticeUrl = "http://w.t3yanzheng.com/XXXXXX";
    // 获取版本号接口
    ET::GetVerUrl = "http://w.t3yanzheng.com/XXXXXX";
    // 心跳验证接口
    ET::HeartBeatUrl = "http://w.t3yanzheng.com/XXXXXX";
    
    std::cout << C::黄 << "获取公告例子" << C::无 << std::endl;
    ET::NOTICE();  // 公告
    分割线();
    
    std::cout << C::黄 << "获取版本号例子" << C::无 << std::endl;
    int VER = ET::GetVersion();
    if (VER == -1) return 1;  // 版本号为-1，代表获取版本号失败
    std::cout << "版本号为: " << VER << std::endl;
    分割线();
    
    std::cout << C::黄 << "卡密验证例子" << C::无 << std::endl;
    std::cout << C::黄 << "正常输入模式" << C::无 << std::endl;
    if (!ET::LOGIN()) return 1;
    std::cout << C::黄 << "隐私输入模式" << C::无 << std::endl;
    ET::卡密隐私输入 = true;  // 打开隐私输入模式
    if (!ET::LOGIN()) return 1;
    分割线();
    
    std::cout << C::黄 << "心跳验证例子" << C::无 << std::endl;
    ET::HeartBeat(true);  // 开始心跳验证
    分割线();
    
    // 这里就是你们的外挂的那个循环，我这里只模拟一下
    for (int i = 10; i != 0 ; i--) {
        std::cout << "\033[90m主线程在" << i * 30 << "秒后结束\033[0m" << std::endl;
        sleep(30);
    }
    
    // 正常退出循环是能执行到这里的
    // 你们那些个二改的源码，大概率不行
    // 因为那些是直接在循环中exit
    // 也有可能是线程处理垃圾导致异常终止
    // 所有这个东西可有可无...吧?
    ET::HeartBeat(false);  // 终止心跳验证

    return 0;
}
/**     ET-T3 变量/函数使用介绍
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *              变量
 * 变量名                类型         介绍                       默认值
 * NoticeUrl             string        获取公告使用的API接口         无
 * LoginUrl              string        卡密登录使用的API接口         无
 * GetVerUrl             string       获取版本号使用的API接口        无
 * HeartBeatUrl          string       心跳验证使用的API接口          无
 * 
 * HeartBeatinterval      int          心跳验证的间隔时间(秒)          10
 * 
 * APPKEY             string         APPKEY                      无
 * RC4KEY             string         RC4密钥                      无
 * 
 * 返回值加密           bool          返回值加密                    false
 * 请求值加密           bool          请求值加密                    false
 * 卡密隐私输入         bool          在输入卡密时使用*代替         false
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *              函数
 * 函数名            可用传参         介绍
 * NOTICE()           0(无)          获取公告(需要先设置NoticeUrl变量)
 * GetVersion()        0(无)          获取版本号(需要先设置GetVerUrl变量)
 * NOTICE()           0(无)          获取公告(需要先设置NoticeUrl变量)
 * LOGIN()            0(无)          进行卡密登录(需要先设置LoginUrl变量)
 * HeartBeat()        1(bool类)       创建/销毁心跳验证(需要先设置HeartBeatUrpl变量)
 *
 * HeartBeat(true)      创建心跳验证
 * HeartBeat(false)     销毁心跳验证
 * 
 *              函数/变量使用方式
 *      ET:: + [变量/函数名]，具体见上方例子
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 *              T3验证管理端设置
 * 机器码验证    开
 * IP验证        开
 * 传输配置->APPKEY (复制到本例子 ET::APPKEY)
 * 传输配置->登录状态码有效期 (推荐3600~10800，过低会导致心跳验证频繁过期)
 * 传输配置->全局数据加解密    开
 * 传输配置->加密算法         Rc4算法
 * 传输配置->Rc4算法加密密钥 (复制到本例子 ET::RC4KEY)
 * 传输配置->请求值加密       (与本程序 ET::请求值加密 保持一致)
 * 传输配置->请求值编码       HEX编码(16进制)
 * 传输配置->返回值加密       (与本程序 ET::返回值加密 保持一致)
 * 传输配置->时间戳检验       开启
 * 传输配置->时间戳校验增强   开启
 * 传输配置->签名校验         双向签名
 * 传输配置->返回值格式       JSON
 * 传输配置->JSON_CODE类型 int
 * 
 * 接口列表->单码卡密登录         复制到本例子 ET::NoticeUrl
 * 接口列表->获取程序公告         复制到本例子 ET::LoginUrl
 * 接口列表->获取最新版本号       复制到本例子 ET::GetVerUrl
 * 接口列表->单码卡密心跳验证     复制到本例子 ET::HeartBeatUrl
 *
 * 其余参数自行选择~
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 * 变量设置优先级
 * 最先: ET::返回值加密, ET::请求值加密, ET::APPKEY, ET::RC4KEY
 * 其次: ET::NoticeUrl, ET::LoginUrl, ET::GetVerUrl, ET::HeartBeatUrl
 * 再次: ET::HeartBeatinterval, ET::卡密隐私输入
 * 
 * 操作优先级
 * 首先设置以下变量
 * ET::返回值加密, ET::请求值加密, ET::APPKEY, ET::RC4KEY
 * (在返回值&请求值加密全部开启情况下，APPKEY和RC4KEY都应正确设置)
 * 其余需自行测试，我懒得讲解逻辑了，不开这两个还不如去用微验(此处无恶意，不要公鸡我)
 * 
 * 然后，是函数和变量对照表
 * 
 * 需要先设置变量，才能调用函数，否则会失败
 * 变量                       函数
 * ET::NoticeUrl              ET::NOTICE()
 * ET::LoginUrl               ET::LOGIN()
 * ET::GetVerUrl              ET::GetVersion()
 * ET::HeartBeatUrl           ET::HeartBeat()
 * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 * 
 *                  Written By F_Error11 on 2025-02-17
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 */