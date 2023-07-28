In the Frameworks you have all the frameworks from current version

In ServerSide is the side that would allow you to upload the framework from web-site

In Test you actually have the programing:

    If you starting from scratch:
        then download the platformio.ini,main.cpp, web_page.h that are in ./Test/src,extra_sript.py last thing is to create your own version.txt file in the same directory as is extra_scripts.py

    If you are adding this to your program:
        Add the dependencies in platformio.ini add extra_scripts and upload_build settings. Add all the stuff from extra_script.py into your own extra_script.py