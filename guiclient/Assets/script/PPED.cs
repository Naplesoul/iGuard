using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net.Sockets;
using System.Net;
using System.Threading;
using System.Text;



[System.Serializable]
public class WebPack_PPE{    
    public bool has_goggle;
    public bool has_glove;
    public bool has_helmet;
}
public class PPED : MonoBehaviour
{
    // Start is called before the first frame update

    public static bool has_goggle = false;
    public static bool has_glove = false;
    public static bool has_helmet = false;
    private static string ip = "0.0.0.0";
    private static int port = 50005;

    private int pack_size = 10240;
    private static Socket socket;

    void Start()
    {
        socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        socket.Bind(new IPEndPoint(IPAddress.Parse(ip), port));
        Thread threadReceive = new Thread(new ThreadStart(receiveFromClient));
        threadReceive.Start();
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    void receiveFromClient(){
        int recv;
        byte[] data;
        IPEndPoint sender = new IPEndPoint(IPAddress.Any, 0);
        EndPoint Remote = (EndPoint)(sender);
        while(true){
            data = new byte[pack_size];
            recv = socket.ReceiveFrom(data, ref Remote);
            //Debug.Log(recv.ToString() + " bytes received from " + Remote.ToString() + ":");
            string node_info_str = Encoding.UTF8.GetString(data, 0, recv);
            //Debug.Log(node_info_str);
            WebPack_PPE node_info = JsonUtility.FromJson<WebPack_PPE>(node_info_str);

            has_glove = node_info.has_glove;
            has_goggle = node_info.has_goggle;
            has_helmet = node_info.has_helmet;
        }
    }

}
