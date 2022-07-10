using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;
using System.Text;
using System.Threading;
using UnityEngine;
using UnityEngine.UI;
using System.IO;
using System;

[System.Serializable]
public class KeyPack_Vector3{
    public float x;
    public float y;
    public float z;
}

[System.Serializable]
public class KeyPack_NodeInfo{
    public int node_num;
    public string pose;
    public bool danger;
    public KeyPack_Vector3[] nodes;
}


[System.Serializable]
public class WebPack_Vector3{
    public float x;
    public float y;
    public float z;
    public float score;
}

[System.Serializable]
public class WebPack_NodeInfo{
    public int node_num;
    public int camera_id;
    public long frame_id;
    public WebPack_Vector3[] nodes;
}
public class Skeleton : MonoBehaviour
{
    private static string ip = "0.0.0.0";
    private static int port = 50002;
    private static Socket socket;

    public GameObject[] nodes;
    public GameObject[] bones;

    private Vector2Int[] bnMap;
    private long last_frame_id;
    private float max_endurable_dist;
    public Vector3[] nodePos, lastNodePos;
    public GameObject nodePrefab;
    //public GameObject kPrefab;
    public GameObject pPrefab;
    public GameObject bonePrefab;

    //public GameObject[] keyNodes;
    public Vector3[] keyPose;
    public GameObject[] keyBones;

    public GameObject[] preNodes;
    public Vector3[] prePose;

    private string[] keyPoseList;
    private int currKeyPoseIndex;

    public Material keyMat;
    public Material defMat;
    public Material preMat;
    public Material dangerMat;
    public Text dangerText;
    public Text simiText;
    public Text nextText;
    public Text timeText;
    public Text hideText;

    public GameObject lathe;
    public GameObject driller;
    private GameObject currentObject;

    private KeyPack_Vector3[][][] savePos;

    // Start is called before the first frame update
    void Start()
    {
        nodes = new GameObject[25];
        bones = new GameObject[18];
        nodePos = new Vector3[25];
        lastNodePos = new Vector3[25];
        bnMap = new Vector2Int[18];
        keyPose = new Vector3[25];
        //keyNodes = new GameObject[25];
        keyBones = new GameObject[18];
        keyPoseList = new string[0];
        prePose = new Vector3[25];
        preNodes = new GameObject[25];
        currKeyPoseIndex = 0;
        last_frame_id = -1;
        max_endurable_dist = 0.5f;
        
        //GameObject.Find("xbot").SetActive(false);
        for (int i = 1; i < 25 ;i ++){
            if (i == 10 || i == 16 || i == 20 || i == 24) continue;
            nodes[i] = GameObject.Instantiate(nodePrefab, new Vector3(0, -10, 0), new Quaternion());
            nodes[i].name = "node-" + i;
            //keyNodes[i] = GameObject.Instantiate(kPrefab, new Vector3(0, -10, 0), new Quaternion());
            preNodes[i] = GameObject.Instantiate(pPrefab, new Vector3(0, -10, 0), new Quaternion());
            preNodes[i].name = "pNode-" + i;
        }
        nodes[0] = null;
        nodes[10] = null;
        nodes[16] = null;
        nodes[20] = null;
        nodes[24] = null;
        for (int i = 0; i < 18 ;i ++){
            bones[i] = GameObject.Instantiate(bonePrefab, new Vector3(0, -10, 0), new Quaternion());
            bones[i].name = "bone-" + i;
            keyBones[i] = GameObject.Instantiate(bonePrefab, new Vector3(0, -10, 0), new Quaternion());
            keyBones[i].GetComponent<MeshRenderer>().material = keyMat;
        }
        bnMap[0] = new Vector2Int(1, 2);
        bnMap[1] = new Vector2Int(2, 5);
        bnMap[2] = new Vector2Int(5, 3);
        bnMap[3] = new Vector2Int(3, 4);
        bnMap[4] = new Vector2Int(5, 6);
        bnMap[5] = new Vector2Int(6, 7);
        bnMap[6] = new Vector2Int(7, 8);
        bnMap[7] = new Vector2Int(8, 9);
        bnMap[8] = new Vector2Int(11, 12);
        bnMap[9] = new Vector2Int(12, 13);
        bnMap[10] = new Vector2Int(13, 14);
        bnMap[11] = new Vector2Int(14, 15);
        bnMap[12] = new Vector2Int(4, 17);
        bnMap[13] = new Vector2Int(17, 18);
        bnMap[14] = new Vector2Int(18, 19);
        bnMap[15] = new Vector2Int(4, 21);
        bnMap[16] = new Vector2Int(21, 22);
        bnMap[17] = new Vector2Int(22, 23);

        CaseChanged(0);
        
        socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        socket.Bind(new IPEndPoint(IPAddress.Parse(ip), port));
        Thread threadReceive = new Thread(new ThreadStart(receiveFromClient));
        threadReceive.Start();

        Debug.Log("server initialized successfully!");
    }

    // Update is called once per frame
    void Update()
    {
        for (int i = 0; i < 25 ;i ++){
            if (nodes[i] == null) continue;
            nodes[i].transform.localPosition = nodePos[i];
            preNodes[i].transform.localPosition = prePose[i];
        }
        for (int i = 0; i < 18 ;i ++){
            if(bones[i] == null) continue;
            Vector3 a = nodePos[bnMap[i].x], b = nodePos[bnMap[i].y];
            Vector3 forward = new Vector3(1, 1, 1);
            Vector3 upward = b - a;
            if (upward.x != 0){
                forward.x = -(upward.y + upward.z)/upward.x;
            }else if (upward.y != 0){
                forward.y = -(upward.x + upward.z)/upward.y;
            }else if (upward.z != 0){
                forward.z = -(upward.y + upward.x)/upward.z;
            }
            
            bones[i].transform.localPosition = (a + b) / 2;
            bones[i].transform.localRotation = Quaternion.LookRotation(forward, upward);
            bones[i].transform.localScale = new Vector3(0.02f, upward.magnitude / 2, 0.02f); 
        }

        float cos = GetSimilarity(keyPose, false);
        simiText.text = "当前步骤完成度：" + Mathf.Round(cos * 10000) / 100 + "%";
        if (cos > 0.94){
            nodes[1].GetComponent<MeshRenderer>().material = keyMat;
            NextKeyPos();
        }else{
            nodes[1].GetComponent<MeshRenderer>().material = defMat;
        }

        Camera.main.transform.localPosition = new Vector3(nodePos[3].x - 1.8f, nodePos[3].y + 1.2f, nodePos[3].z - 0.8f);
        timeText.text = string.Format("{0:T}", DateTime.Now);
    }

    private float GetSimilarity(Vector3[] kp, bool strict){
        Vector3 m = new Vector3();
        float r = 0;
        if (!strict){
            for (int i = 0; i < 25 ;i ++){
                if (nodes[i] == null) continue;
                m += keyPose[i] - nodePos[i];
            }
            m = m / 20;
        }
        float m1 = 0, m2 = 0;
        // for (int i = 0; i < 25 ;i ++){
        //     if (nodes[i] == null) continue;
        //     r += (nodePos[i] + m).x * (kp[i].x);
        //     r += (nodePos[i] + m).y * (kp[i].y);
        //     r += (nodePos[i] + m).z * (kp[i].z);
        //     m1 += (nodePos[i] + m).magnitude * (nodePos[i] + m).magnitude;
        //     m2 += kp[i].magnitude * kp[i].magnitude;
        // }
        for (int i = 0; i < 18; i ++){
            Vector3 a = nodePos[bnMap[i].x], b = nodePos[bnMap[i].y];
            Vector3 ak = kp[bnMap[i].x], bk = kp[bnMap[i].y];
            Vector3 upward = (b - a), upwardk = (bk - ak).normalized * (b - a).magnitude;
            r += upward.x * upwardk.x;
            r += upward.y * upwardk.y;
            r += upward.z * upwardk.z;
            m1 += upward.magnitude * upward.magnitude;
            m2 += upwardk.magnitude * upwardk.magnitude;
        }
        float cos = r / Mathf.Sqrt(m1) / Mathf.Sqrt(m2);
        return cos * cos;
    }

    private void NextKeyPos(){
        currKeyPoseIndex ++;
        if (currKeyPoseIndex < keyPoseList.Length){
            LoadKeyPos(keyPoseList[currKeyPoseIndex]);
        }else {
            nextText.text = "无关键步骤信息";
            foreach (Transform child in currentObject.transform)
            {
                if(child.gameObject.name.Contains("danger")){
                    CapsuleCollider collider = child.gameObject.GetComponent<CapsuleCollider>();
                    collider.enabled = false;
                    child.gameObject.GetComponent<MeshRenderer>().material = preMat;
                    break;
                }
            }
        }
    }

    private void LoadKeyPos(string filename){
        Debug.Log("loading key pose " + filename);
        StreamReader sr = new StreamReader("./Assets/Pose/" + filename + ".json", Encoding.UTF8);
        string content =  sr.ReadToEnd();
        sr.Close();
        KeyPack_NodeInfo node_info = JsonUtility.FromJson<KeyPack_NodeInfo>(content);
        for (int i = 0; i < 25 ;i ++){
            keyPose[i].x = node_info.nodes[i].x / 1000;
            keyPose[i].y = node_info.nodes[i].y / 1000;
            keyPose[i].z = node_info.nodes[i].z / 1000;
        }
        foreach (Transform child in currentObject.transform)
        {
            if(child.gameObject.name.Contains("danger")){
                CapsuleCollider collider = child.gameObject.GetComponent<CapsuleCollider>();
                if (node_info.danger){
                    collider.enabled = true;
                    child.gameObject.GetComponent<MeshRenderer>().material = dangerMat;
                }else{
                    collider.enabled = false;
                    child.gameObject.GetComponent<MeshRenderer>().material = preMat;
                }
                break;
            }
        }
        nextText.text = "请执行关键步骤("+ (currKeyPoseIndex + 1) +"/"+ keyPoseList.Length +")：" + node_info.pose;
    }

    private void SetNodePos(WebPack_NodeInfo ni){
        if (ni.node_num != 25) return;

        if (last_frame_id > ni.frame_id) return;

        if (last_frame_id == ni.frame_id){
            // if (ni.camera_id == 1){
            //     for (int i = 0; i < 25 ;i ++){
            //         Vector3 nodepos = new Vector3(ni.nodes[i].x / 1000, ni.nodes[i].y / 1000, ni.nodes[i].z / 1000);
            //         if ((nodepos - lastNodePos[i]).magnitude <= max_endurable_dist){
            //             nodePos[i] = nodepos;
            //         }
            //     }
            // }else {
            //     for (int i = 0; i < 25 ;i ++){
            //         Vector3 nodepos = new Vector3(ni.nodes[i].x / 1000, ni.nodes[i].y / 1000, ni.nodes[i].z / 1000);
            //         if ((lastNodePos[i] - nodePos[i]).magnitude > max_endurable_dist){
            //             nodePos[i] = nodepos;
            //         }
            //     }
            // }

            for (int i = 0; i < 25 ;i ++){
                Vector3 nodepos = new Vector3(ni.nodes[i].x / 1000, ni.nodes[i].y / 1000, ni.nodes[i].z / 1000);
                float dist = (nodePos[i] - nodepos).magnitude;
                if (dist <= max_endurable_dist){
                    nodePos[i] = (nodePos[i] + nodepos) / 2;
                }else {
                    if ((lastNodePos[i] - nodePos[i]).magnitude > (nodepos - lastNodePos[i]).magnitude){
                        nodePos[i] = nodepos; 
                    }
                }
            }
        }else{
            for (int i = 0; i < 25 ;i ++){
                lastNodePos[i] = nodePos[i];
                //if (ni.nodes[i].score == 0) continue;
                // nodePos[i].x = nodePos[i].x * (1 - ni.nodes[i].score) + (ni.nodes[i].x / 1000 * ni.nodes[i].score);
                // nodePos[i].y = nodePos[i].y * (1 - ni.nodes[i].score) + (ni.nodes[i].y / 1000 * ni.nodes[i].score);
                // nodePos[i].z = nodePos[i].z * (1 - ni.nodes[i].score) + (ni.nodes[i].z / 1000 * ni.nodes[i].score);
                nodePos[i].x = ni.nodes[i].x / 1000;
                nodePos[i].y = ni.nodes[i].y / 1000;
                nodePos[i].z = ni.nodes[i].z / 1000;
            }
            if (last_frame_id != -1){
                for (int i = 0; i < 25 ;i ++){
                    prePose[i].x = (nodePos[i].x - lastNodePos[i].x) < 0.1 ? (2 * nodePos[i].x - lastNodePos[i].x) : lastNodePos[i].x;
                    prePose[i].y = (nodePos[i].y - lastNodePos[i].y) < 0.1 ? (2 * nodePos[i].y - lastNodePos[i].y) : lastNodePos[i].y;
                    prePose[i].z = (nodePos[i].z - lastNodePos[i].z) < 0.1 ? (2 * nodePos[i].z - lastNodePos[i].z) : lastNodePos[i].z;
                }
            }
            last_frame_id = ni.frame_id;
        }
    }

    void receiveFromClient(){
        int recv;
        byte[] data;
        IPEndPoint sender = new IPEndPoint(IPAddress.Any, 0);
        EndPoint Remote = (EndPoint)(sender);
        while(true){
            data = new byte[10240];
            recv = socket.ReceiveFrom(data, ref Remote);
            //Debug.Log(recv.ToString() + " bytes received from " + Remote.ToString() + ":");
            string node_info_str = Encoding.UTF8.GetString(data, 0, recv);
            Debug.Log(node_info_str);

            WebPack_NodeInfo node_info = JsonUtility.FromJson<WebPack_NodeInfo>(node_info_str);
            //Debug.Log(node_info.node_num);
            //Debug.Log(node_info.nodes);
            SetNodePos(node_info);
        }
    }
    public void NewKeyPose(){

        // for (int i = 1; i < 25 ;i ++){
        //     if (nodes[i] == null) continue;
        //     GameObject.Instantiate(kPrefab, nodePos[i], new Quaternion());
        //     keyPose[i] = nodePos[i];
        // }

        // for (int i = 0; i < 18 ;i ++){
        //     if (bones[i] == null) continue;
        //     bones[i].GetComponent<MeshRenderer>().material = keyMat;
        //     bones[i] = GameObject.Instantiate(bonePrefab);
        // }

        // for (int i = 0; i < 25 ;i ++){
        //     if (nodes[i] == null) continue;
        //     keyNodes[i].transform.localPosition = nodePos[i];
        //     keyPose[i] = nodePos[i];
        // }
        // for (int i = 0; i < 18 ;i ++){
        //     if(bones[i] == null) continue;
        //     Vector3 a = nodePos[bnMap[i].x], b = nodePos[bnMap[i].y];
        //     Vector3 forward = new Vector3(1, 1, 1);
        //     Vector3 upward = b - a;
        //     if (upward.x != 0){
        //         forward.x = -(upward.y + upward.z)/upward.x;
        //     }else if (upward.y != 0){
        //         forward.y = -(upward.x + upward.z)/upward.y;
        //     }else if (upward.z != 0){
        //         forward.z = -(upward.y + upward.x)/upward.z;
        //     }
            
        //     keyBones[i].transform.localPosition = (a + b) / 2;
        //     keyBones[i].transform.localRotation = Quaternion.LookRotation(forward, upward);
        //     keyBones[i].transform.localScale = new Vector3(0.02f, upward.magnitude / 2, 0.02f); 
        // }
    }

    public void CaseChanged(int value){
        Debug.Log("Change to mode " + value);
        lathe.SetActive(value == 0);
        driller.SetActive(value == 1);
        if (value == 0){
            currentObject = lathe;
            keyPoseList = new string[5]{"lathe/tighten", "lathe/start", "lathe/cut", "lathe/stop", "lathe/release"};
            currKeyPoseIndex = -1;
            NextKeyPos();
        }else if (value == 1){
            currentObject = driller;
            keyPoseList = new string[5]{"driller/tighten", "driller/start", "driller/drill", "driller/stop", "driller/release"};
            currKeyPoseIndex = -1;
            NextKeyPos();
        }
    }

    public void DangerModeChanged(bool f){
        dangerText.enabled = f;
        for (int i = 0; i < 25 ;i ++){
            if (nodes[i] == null) continue;
            nodes[i].GetComponent<SphereCollider>().enabled = f;
        }
    }

    public void KeyModeChanged(bool f){
        simiText.enabled = f;
        nextText.enabled = f;
        if (f){
            currKeyPoseIndex = -1;
            NextKeyPos();
        }
    }
}
