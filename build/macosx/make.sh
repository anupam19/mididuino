#!/bin/sh


### -- SETUP WORK DIR -------------------------------------------

DISTNAME=Mididuino
RESOURCES=`pwd`/work/${DISTNAME}.app/Contents/Resources/Java

HARDWAREDIR=../../hardware
DISTDIR=../../../mididuino-dist
TOOLSZIP=${DISTDIR}/macosx/tools-universal.zip
SHAREDDIST=${DISTDIR}/shared

#echo $RESOURCES
#exit

if test -d work
then
  BUILD_PREPROC=false
else
  echo Setting up directories to build under Mac OS X
  BUILD_PREPROC=true

  mkdir work

  # to have a copy of this guy around for messing with
  echo Copying ${DISTNAME}.app...
  #cp -a dist/${DISTNAME}.app work/   # #@$(* bsd switches
  #/sw/bin/cp -a dist/${DISTNAME}.app work/
  cp -pRX ${DISTDIR}/macosx/Application.app work/${DISTNAME}.app
	PREVDIR=`pwd`
	cd ../../hardware/tools/mididuino && make -f Makefile
	cd "${PREVDIR}"
	
  # cvs doesn't seem to want to honor the +x bit 
  chmod +x work/${DISTNAME}.app/Contents/MacOS/JavaApplicationStub

  cp -rX ../../app/lib "$RESOURCES/"
#  cp -rX ../shared/libraries "$RESOURCES/"
#  cp -rX ../shared/tools "$RESOURCES/"
  
  cp -rX "$HARDWAREDIR" "$RESOURCES/"

  cp -X ../../app/lib/antlr.jar "$RESOURCES/"
#  cp -X ../../app/lib/ecj.jar "$RESOURCES/"
  cp -X ../../app/lib/jna.jar "$RESOURCES/"
  cp -X ../../app/lib/oro.jar "$RESOURCES/"

#  echo Copying examples...
#  cp -r ../shared/examples "$RESOURCES/"

#  echo Extracting reference...
#  unzip -q -d "$RESOURCES/" ../shared/reference.zip
  
#  echo Extracting avr tools...
  unzip -q -d "$RESOURCES/" "$TOOLSZIP"

#  mv "${RESOURCES}/hardware/tools/mididuino" "${RESOURCES}/tools"
fi


### -- START BUILDING -------------------------------------------

# move to root 'processing' directory
cd ../..


### -- BUILD CORE ----------------------------------------------

echo Building processing.core...

cd core

#CLASSPATH=/System/Library/Frameworks/JavaVM.framework/Classes/classes.jar:/System/Library/Frameworks/JavaVM.framework/Classes/ui.jar:/System/Library/Java/Extensions/QTJava.zip
#export CLASSPATH

perl preproc.pl

mkdir -p bin
javac -source 1.5 -target 1.5 -d bin \
  src/processing/core/*.java \
  src/processing/xml/*.java

rm -f "$RESOURCES/core.jar"

cd bin && \
		zip -rq "$RESOURCES/core.jar" \
  processing/core/*.class \
  processing/xml/*.class
cd ..

# head back to "processing/app"
echo `pwd`
cd ../app 
echo `pwd`



### -- BUILD PDE ------------------------------------------------

echo Building the PDE...

# For some reason, javac really wants this folder to exist beforehand.
rm -rf ../build/macosx/work/classes
mkdir ../build/macosx/work/classes
# Intentionally keeping this separate from the 'bin' folder
# used by eclipse so that they don't cause conflicts.

javac \
    -Xlint:deprecation \
    -source 1.5 -target 1.5 \
    -classpath "$RESOURCES/core.jar:$RESOURCES/antlr.jar:$RESOURCES/ecj.jar:$RESOURCES/jna.jar:$RESOURCES/oro.jar" \
    -d ../build/macosx/work/classes \
    src/processing/app/*.java \
    src/processing/app/debug/*.java \
    src/processing/app/macosx/*.java \
    src/processing/app/preproc/*.java \
    src/processing/app/syntax/*.java \
    src/processing/app/tools/*.java \
    src/processing/app/library/*.java

cd ../build/macosx/work/classes
rm -f "$RESOURCES/pde.jar"
zip -0rq "$RESOURCES/pde.jar" .
cd ../..

# get updated core.jar and pde.jar; also antlr.jar and others
#mkdir -p work/${DISTNAME}.app/Contents/Resources/Java/
#cp work/lib/*.jar work/${DISTNAME}.app/Contents/Resources/Java/


echo
echo Done.
