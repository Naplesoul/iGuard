import os
import sys
from bs4 import BeautifulSoup

html = "./dataset/raw/mocap.html"
out_dir = "./dataset/meta"

def find_all(filename, key):
    f = open(filename, "r")

    data = f.read()

    soup = BeautifulSoup(data, "html.parser")

    trs = soup.select("tr")

    subject_num = "-1"
    sequnce_num = "-1"
    subject_name = ""
    res = []

    for tr in trs:
        if tr.text.find("Subject #") >= 0:
            subject_name = tr.select("td")[0].text
            subject_name = subject_name[subject_name.find("(") + 1 : subject_name.find(")")]
            subject_num = tr.text[tr.text.find("Subject #") + 9:tr.text.find("(")].strip()
            if len(subject_num) == 1:
                subject_num = "0" + subject_num
        if tr.text.find("Feedback") >= 0 and (tr.text.find("120") >= 0 or tr.text.find("60") >= 0) and tr.text.find(key) >= 0:
            sequnce_num = tr.select("td")[1].text
            if len(sequnce_num) == 1:
                sequnce_num = "0" + sequnce_num
            description = tr.select("td")[2].text

            # remove last "\n"
            if description[len(description) - 1] == "\n":
                description = description[:len(description) - 1]
            res.append([subject_num + "-" + sequnce_num, int(tr.select("td")[8].text), description, subject_name])
    f.close()
    return res

if __name__ == "__main__":
    os.makedirs(out_dir, exist_ok=True)
    classname = sys.argv[len(sys.argv) - 1]
    results = find_all(html, classname)

    # every result list should be in one line
    # for better readability
    jsonstr = "[\n"
    for result in results:
        jsonstr += "\t[\"{}\", {}, \"{}\", \"{}\"],\n".format(result[0], result[1], result[2], result[3])
    if jsonstr[len(jsonstr) - 2] == ",":
        jsonstr = jsonstr[:len(jsonstr) - 2] + "\n]"
    
    f = open(os.path.join(out_dir, classname + ".json"), "w+")
    f.write(jsonstr)
    f.close()