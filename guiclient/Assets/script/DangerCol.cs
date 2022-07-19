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


    private static string ip = "192.168.0.104";

    private static int port = 40790;
    public static Socket socket;

    public string level;
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
            dangerText.text = "！！误触" + level + "级危险区：" + this.name;
            SendMsg("<" + level);
        }else if (other.gameObject.name.Contains("pNode")){
            dangerText.text = "可能进入危险区：" + other.gameObject.name;
        }
    }

    private void OnCollisionEnter(Collision other) {
        Debug.Log("Danger!");
        if (other.gameObject.name.Contains("node")){
            alert_id = Alert.addAlertMsg("请离开危险区：" + this.name, (level[0] - '@') * 10);
        }
    }

    private void OnCollisionExit(Collision other) {
        if (dangerText.text.Contains(other.gameObject.name)){
            dangerText.text = "当前无危险";
            SendMsg(">" + level);
        }
        if (other.gameObject.name.Contains("node")){
            Alert.removeAlertMsg(alert_id);
            alert_id = -1;
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
