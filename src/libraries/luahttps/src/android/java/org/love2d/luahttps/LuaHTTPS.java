package org.love2d.luahttps;

import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.Keep;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@Keep
class LuaHTTPS {
    static private String TAG = "LuaHTTPS";

    private String urlString;
    private String method;
    private byte[] postData;
    private byte[] response;
    private int responseCode;
    private HashMap<String, String> headers;

    public LuaHTTPS() {
        headers = new HashMap<String, String>();
        reset();
    }

    public void reset() {
        urlString = null;
        method = "GET";
        postData = null;
        response = null;
        responseCode = 0;
        headers.clear();
    }

    @Keep
    public void setUrl(String url) {
        urlString = url;
    }

    @Keep
    public void setPostData(byte[] postData) {
        this.postData = postData;
    }

    @Keep
    public void setMethod(String method) {
        this.method = method.toUpperCase();
    }

    @Keep
    public void addHeader(String key, String value) {
        headers.put(key, value);
    }

    @Keep
    public String[] getInterleavedHeaders() {
        ArrayList<String> resultInterleaved = new ArrayList<String>();

        for (Map.Entry<String, String> header: headers.entrySet()) {
            String key = header.getKey();
            String value = header.getValue();

            if (key != null && value != null) {
                resultInterleaved.add(key);
                resultInterleaved.add(value);
            }
        }

        String[] result = new String[resultInterleaved.size()];
        resultInterleaved.toArray(result);
        return result;
    }

    @Keep
    public int getResponseCode() {
        return responseCode;
    }

    @Keep
    public byte[] getResponse() {
        return response;
    }

    @Keep
    public boolean request() {
        if (urlString == null) {
            return false;
        }

        URL url;
        try {
            url = new URL(urlString);

            if (!url.getProtocol().equals("http") && !url.getProtocol().equals("https")) {
                return false;
            }
        } catch (MalformedURLException e) {
            Log.e(TAG, "Error", e);
            return false;
        }

        HttpURLConnection connection;
        try {
            connection = (HttpURLConnection) url.openConnection();
        } catch (IOException e) {
            Log.e(TAG, "Error", e);
            return false;
        }

        // Set request method
        try {
            connection.setRequestMethod(method);
        } catch (ProtocolException e) {
            Log.e(TAG, "Error", e);
            return false;
        }

        // Set header
        for (Map.Entry<String, String> headerData: headers.entrySet()) {
            connection.setRequestProperty(headerData.getKey(), headerData.getValue());
        }

        // Set post data
        if (postData != null && canSendData()) {
            connection.setDoOutput(true);

            try {
                OutputStream out = connection.getOutputStream();
                out.write(postData);
            } catch (Exception e) {
                Log.e(TAG, "Error", e);
                connection.disconnect();
                return false;
            }
        }

        // Request
        try {
            InputStream in;

            // Set response code
            responseCode = connection.getResponseCode();
            if (responseCode >= 400) {
                in = connection.getErrorStream();
            } else {
                in = connection.getInputStream();
            }

            // Read response
            int readed;
            byte[] temp = new byte[4096];
            ByteArrayOutputStream response = new ByteArrayOutputStream();

            while ((readed = in.read(temp)) != -1) {
                response.write(temp, 0, readed);
            }

            this.response = response.toByteArray();
            response.close();

            // Read headers
            headers.clear();
            for (Map.Entry<String, List<String>> header: connection.getHeaderFields().entrySet()) {
                headers.put(header.getKey(), TextUtils.join(", ", header.getValue()));
            }
        } catch (Exception e) {
            Log.e(TAG, "Error", e);
            connection.disconnect();
            return false;
        }

        connection.disconnect();
        return true;
    }

    private boolean canSendData() {
        return !method.equals("GET") && !method.equals("HEAD");
    }
}
