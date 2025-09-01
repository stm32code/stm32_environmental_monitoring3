package com.example.smartweatherclock.utils

import android.content.Context
import android.util.Log
import com.example.smartweatherclock.entity.Send
import com.google.gson.Gson
import com.itfitness.mqttlibrary.MQTTHelper

object Common {

//    const val PORT = "6002" // mqtt服务器端口号
//    const val SERVER_ADDRESS = "183.230.40.39"// mqtt 服务器地址


    const val PORT = "1883" // mqtt服务器端口号
    const val SERVER_ADDRESS = "8.133.197.10"// mqtt 服务器地址
    const val URL = "tcp://$SERVER_ADDRESS:$PORT" // mqtt连接地址
    const val RECEIVE_TOPIC = "/broadcast/964/test2" // 接收消息订阅的主题 - 下位机发送消息的主题
    const val PUSH_TOPIC = "/broadcast/964/test1" // 推送消息的主题 - 下位机接收消息的主题
    const val DRIVER_ID =
        "96-4-app" // mqtt id
    const val DRIVER_NAME = "96-4-app" // mqtt 用户名 （oneNET中为产品ID）
    const val DRIVER_PASSWORD =
        "96-4-app" // mqtt 鉴权或者密码
    const val DRIVER_ID_HARDWARE = "1211070683" // mqtt 硬件id
    const val API_KEY = "HPtgv9tFnmoLlt=suai8ogSEwPg=" // （oneNET） APIkey
    var HARDWARE_ONLINE = false // 硬件在线标志位
    var mqttHelper: MQTTHelper? = null // mqtt 连接服务函数

    const val WEATHER_API = "S9GHROeh4KtVCJ00M"

    /***
     * @brief 包装发送函数，只有建立了连接才发送消息
     */
    fun sendMessage(context: Context, cmd: Int, vararg data: String): String {
        return if (mqttHelper == null || !mqttHelper!!.connected) {
            MToast.mToast(context, "未建立连接")
            ""
        } else {
            try {
                val send = Send(cmd = cmd)
                when (cmd) {
                    1 -> {
                        send.time = data[0]
                    }

                    2 -> {
                        send.time1 = data[0]
                        send.time2 = data[1]
                    }

                    3 -> {
                        send.time1 = data[0]
                        send.time2 = data[1]
                    }

                    4 -> {
                        send.city = data[0]
                    }

                }
                val result = Gson().toJson(send)
                mqttHelper!!.publish(PUSH_TOPIC, result, 1)
                result
            } catch (e: Exception) {
                e.printStackTrace()
                Log.e("发送错误", e.message.toString())
                MToast.mToast(context, "数据发送失败")
                ""
            }
        }
    }

}