const char *web = R""""(
<!DOCTYPE html>
    <head>
        <title>Deice config</title>
    </head>
    <body>
        <form action="/" method="GET">
            <label for="fname">SSID</label>
            <select name="ssid" id="ssid">
            <options>
            </select>
            <br><br>
            <label for="lname">Password</label>
            <input type="text" id="passwd" name="passwd"><br><br>
            <input type="submit" value="Connect">
            </form>
    </body>
</html>
)"""";
const char *configDonePage = R""""(
<!DOCTYPE html>
    <head>
        <title>Deice config</title>
    </head>
    <body>
        <p>Your config is done check if it is connected</p>
    </body>
</html>
)"""";