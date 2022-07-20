using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class Alert : MonoBehaviour
{
    // Start is called before the first frame update

    public Text alertText;
    public GameObject alertPanel;

    private static ArrayList msg_list;
    private static bool needUpdate = false;
    private static object lock_obj = new object();

    private class Message{
        public string text;
        public int emer;

        public int id;

        private static int next_id = 0;

        public Message(string t, int e){
            text = t; emer = e; id = next_id;
            next_id ++;
        }
    } 

    void Start()
    {
        alertPanel.SetActive(false);
        alertText.text = "";
        msg_list = new ArrayList();
    }

    // Update is called once per frame
    void Update()
    {
        lock(lock_obj){
            if (needUpdate){
                if (msg_list.Count > 0){
                    int max_emer = 0;
                    alertPanel.SetActive(true);
                    foreach(Message msg in msg_list){
                        if (msg.emer > max_emer){
                            max_emer = msg.emer;
                            alertText.text = msg.text;
                        }
                    }
                }else{
                    alertPanel.SetActive(false);
                    alertText.text = "";
                }
                needUpdate = false;
            }
        }
    }

    public static int addAlertMsg(string msg, int emer){
        needUpdate = true;
        Message m;
        lock(lock_obj){
            m = new Message(msg, emer);
            msg_list.Add(m);
        }
        return m.id;
    }

    public static int updateAlertMsg(int id, string msg, int emer){
        needUpdate = true;
        lock(lock_obj){
            int len = msg_list.Count;
            for (int i = 0; i < len; i++){
                if (((Message)msg_list[i]).id == id){
                    ((Message)msg_list[i]).text = msg;
                    ((Message)msg_list[i]).emer = emer;
                    return id;
                }
            }
        }
        return addAlertMsg(msg, emer);
    }

    public static void removeAlertMsg(int id){
        if (id < 0)return;
        needUpdate = true;
        lock(lock_obj){
            int len = msg_list.Count;
            for (int i = 0; i < len; i++){
                if (((Message)msg_list[i]).id == id){
                    msg_list.RemoveAt(i);
                    break;
                }
            }
        }
    }
}
