using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Net.Sockets;
using System.Net;
using System.Text;

public class DangerCol : MonoBehaviour
{
    public static Text dangerText;


    private static string ip = "192.169.0.104";

    private static int port = 40790;
    public static Socket socket;

    public string level;
    public string danger_name;
    private int alert_id;


    // Start is called before the first frame update
    void Start()
    {
        alert_id = -1;
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    private void OnCollisionStay(Collision other) {
        if (other.gameObject.name.Contains("node")){
            dangerText.text = "！！误触" + level + "级危险区：" + other.gameObject.name;
            SendMsg("<" + level);
        }else if (other.gameObject.name.Contains("pNode")){
            dangerText.text = "可能进入危险区：" + other.gameObject.name;
        }
    }

    private void OnCollisionEnter(Collision other) {
        //Debug.Log("Danger!");
        if (other.gameObject.name.Contains("node")){
            alert_id = Alert.updateAlertMsg(alert_id, "请离开危险区：\n" + danger_name, (level[0] - '@') * 10);
        }
    }

    private void OnCollisionExit(Collision other) {
        if (dangerText.text.Contains(other.gameObject.name)){
            dangerText.text = "当前无危险";
        }
        if (other.gameObject.name.Contains("node")){
            Alert.removeAlertMsg(alert_id);
            alert_id = -1;
            SendMsg(">" + level);
        }
    }

    static void SendMsg(string msg){
        EndPoint point = new IPEndPoint(IPAddress.Parse(ip), port);
        socket.SendTo(Encoding.UTF8.GetBytes(msg), point);
    }

    public static void noDanger(){
        dangerText.text = "当前无危险";
        SendMsg("  ");
    }
}
