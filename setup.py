import shutil
import os
from sys import platform
import tarfile
from zipfile import ZipFile
import subprocess
path = os.path.dirname(__file__)

# Rutas de Instalacion
linuxInstall="/opt/Cats-Farm"
windowsInstall="C:/Cats-Farm"
macInstall="/usr/local/Cats-Farm"
#--------------------------

def copydir( src, dst ):
	if not os.path.isdir( dst ): 
		shutil.copytree( src, dst )

def copyfile( src, dst ):
	if not os.path.isfile( dst ): 
		shutil.copy( src, dst )

def compile_ ( project ):
	if platform == "linux2":
		qmake = "/opt/Qt5.7.1/5.7/gcc_64/bin/qmake"
	else: 
		qmake = "C:/Qt/Qt5.7.1/5.7/mingw53_32/bin/qmake.exe"
		make = "C:/Qt/Qt5.7.1/Tools/mingw530_32/bin/mingw32-make.exe"

	dirname = os.path.dirname(project).replace("\\","/")
	basename = os.path.basename(dirname)

	print "Compiling " + dirname + "..."



	if platform == "win32":
		exe = dirname + "/" + basename + ".exe"

		if os.path.isfile(exe): os.remove(exe)

		os.system("cd " + dirname + " && " + qmake + " " + project + " > %temp%/null" )
		os.system( "set path=" + os.path.dirname( make ) + "; && cd " + dirname + " && " + make + " > %temp%/null")

		shutil.move( exe, linuxInstall + "/bin/win/" + basename + ".exe" )



	else:
		exe = dirname + "/" + basename

		if os.path.isfile(exe): os.remove(exe)

		os.system("cd " + dirname +" && " + qmake + " "+project + " &> /dev/null")
		os.system("cd " + dirname + " && source /opt/rh/devtoolset-7/enable && make > /dev/null")

		shutil.move( exe, linuxInstall + "/bin/linux/" + basename )

	print "Finished Compler."

def compiler_install():
	if platform == "win32":
		if not os.path.isdir( "C:/Qt" ):
			zf = ZipFile( path + "/qt5/win/Qt5.7.1.zip", "r" )
			zf.extractall( "C:/" )

	if platform == "linux2":
		if not os.path.isdir( "/opt/Qt5.7.1" ):
			# untar QT5
			tar = tarfile.open( path + "/qt5/linux/Qt5.7.1.tar.gz" )
			for i in tar:
				tar.extract(i, path = "/opt")
			tar.close()
			#----------------------

		# install rpms nesesarios
		rpms = ""
		for rpm in os.listdir( path + "/qt5/linux/libs" ):
			rpms +=  path + "/qt5/linux/libs/" + rpm + " "
		os.system( "yum -y install " + rpms + " &> /dev/null" )
		#----------------------


