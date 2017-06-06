/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.iget.ccnx.adapter;

import java.io.Serializable;
import java.text.SimpleDateFormat;
import java.util.Date;


public class OutputListViewElement implements Serializable{

    String url;
    String savedPath;
    String nameFile;
    String md5;
    int size;
    Date date;

    public OutputListViewElement(String url, String savedPath, String nameFile, String md5, int size) {
        this.url = url;
        this.savedPath = savedPath;
        this.nameFile = nameFile;
        this.md5 = md5;
        this.size = size;
        this.date = new Date();
    }

    public String getUrl() {
        return url;
    }

    public void setUrl(String url) {
        this.url = url;
    }

    public String getSavedPath() {
        return savedPath;
    }

    public void setSavedPath(String savedPath) {
        this.savedPath = savedPath;
    }

    public String getNameFile() {
        return nameFile;
    }

    public void setNameFile(String nameFile) {
        this.nameFile = nameFile;
    }

    public String getMd5() {
        return md5;
    }

    public void setMd5(String md5) {
        this.md5 = md5;
    }

    public int getSize() {
        return size;
    }

    public void setSize(int size) {
        this.size = size;
    }

    public Date getDate() {
        return date;
    }

    public void setDate(Date date) {
        this.date = date;
    }

    public String getDateSting(String format) {
        return new SimpleDateFormat(format).format(date);
    }



}
