Import("env")
from firebase_admin import credentials, initialize_app, storage, db
import json
cred = credentials.Certificate("Credentials.json")
initialize_app(cred, {'storageBucket': 'otaupdates-129df.appspot.com','databaseURL':"https://otaupdates-129df-default-rtdb.europe-west1.firebasedatabase.app/"})

f = open("version.txt", "r")
version=f.read().split("_")
f.close()
for a in range(len(version)):
    version[a]=int(version[a])
buildFile=bool(env.GetProjectOption("upload_build"))
if(buildFile):
    version[2]+=1
    if(version[2]==10):
        version[1]+=1
        version[2]=0
    if(version[1]==10):
        version[0]+=1
        version[1]=0
versionString=""
for a in range(3):
    versionString+=str(version[a])+("_" if a<2 else "")
f=open("version.txt","w")
f.write(versionString)
f.close()

def after_build(source, target,env):
    print("Now we will upload the file to firebase")
    bucket = storage.bucket()
    fileName=str(source[0])
    blob = bucket.blob(("firmware_%s" % versionString)+".bin")
    blob.upload_from_filename(fileName)
    blob.make_public()
    url=blob.public_url
    ref = db.reference("/firmwares")
    file='{"firmware_%s":{"url":"%s","version":%d } }'%(versionString,url,int(versionString.replace("_","")))
    data=json.loads(file)
    ref.update(data)
    ref = db.reference("/")
    file='{"version":"%s"}'%versionString
    data=json.loads(file)
    ref.update(data)

env.AddPostAction("buildprog",after_build)

env.Replace(PROGNAME="firmware_%s" % versionString)