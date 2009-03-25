/**
 * Copyright (c) 2009, Christian Schneider
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * - Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  - Neither the names of the authors nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
package com.ruinwesen.patchdownloader.repository;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Enumeration;

import name.cs.csutils.CSUtils;
import name.cs.csutils.FileFilterFactory;
import name.cs.csutils.iterator.FileEnumeration;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public abstract class LocalRepository extends Repository {

    private Log log = LogFactory.getLog(LocalRepository.class);
    private File repositoryDir;
    private boolean compressed;

    public LocalRepository(File repositoryDir, boolean compressed) {
        this.repositoryDir = repositoryDir;
        this.compressed = compressed;
    }

    public LocalRepository(boolean compressed) {
        this.compressed = compressed;
        this.repositoryDir = null;
    }
    
    public boolean isCompressed() {
        return compressed;
    }
    
    public void validate() throws IOException {
        if (repositoryDir == null) {
            throw new IOException("base directory not set: "+repositoryDir);
        } else if (repositoryDir.isFile()) {
            throw new IOException("base directory is a file: "+repositoryDir);
        } else if (!repositoryDir.exists()) {
            throw new IOException("base directory does not exist");
        }
    }

    public void setBaseDir(File baseDir) {
        this.repositoryDir = baseDir;
    }

    public File getBaseDir() {
        return repositoryDir;
    }

    @Override
    public <C extends StoredPatchCollector> C collectPatches(C collector) {
        try {
            validate();
        } catch (IOException ex) {
            if (log.isErrorEnabled()) {
                log.error("validate() failed", ex);
            }
            return collector;
        }
        
        if (compressed) {
            Enumeration<File> files = new FileEnumeration(repositoryDir, ".rwp");
            while (files.hasMoreElements()) {
                File file = files.nextElement();
                if (file.isDirectory() || 
                        !repositoryDir.equals(file.getParentFile())) {
                    continue;
                }
                if (!collector.takesMore()) {
                    break;
                }
                collector.collect(this, new StoredPatch.JarFilePatch(file));
            }
        } else {
            File[] files = repositoryDir.listFiles(FileFilterFactory.DirectoriesOnly());
            if (files == null) {
                if (log.isErrorEnabled()) {
                    log.error("could not retrieve directories");
                }
                return collector;
            }
            for (File file: files) {
                if (!collector.takesMore()) {
                    break;
                }
                collector.collect(this, new StoredPatch.DirectoryPatch(file));
            }
        }
        return collector;
    }

    @Override
    public void export(StoredPatch patch, OutputStream out) throws IOException {
        if (compressed) {
            StoredPatch.JarFilePatch jp = (StoredPatch.JarFilePatch) patch;
            CSUtils.copy(jp.getFile(), out);
        } else {
            // TODO compress directory into a jar file and export it
            throw CSUtils.notImplementedYet();
        }
    }

}