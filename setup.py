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


def sublime_build():
	if platform == "win32": 
		proc = subprocess.Popen( "%username%", stdout=subprocess.PIPE , stderr=subprocess.PIPE, shell=True)
		user = proc.communicate()[1].split("'")[1]

		dirs = "C:/Users/" + user + "/AppData/Roaming/Sublime Text 3/Packages/User"

		f = open( dirs + "/QT5-C++.sublime-build", "w" )
		f.write( '{ "cmd":["python", "' + dirs + '/QT5-C++.py","${file}"]}' )
		f.close()

		copyfile( path + "/modules/QT5-C++.py", dirs )

	else: 
		user = os.listdir("/home")[0]
		dirs = "/home/" + user + "/.config/sublime-text-3/Packages/User"

		f = open( dirs + "/QT5-C++.sublime-build", "w" )
		f.write( '{ "cmd":["python", "' + dirs + '/QT5-C++.py","${file}"]}' )
		f.close()

		copyfile( path + "/modules/QT5-C++.py", dirs )

		os.system( "chmod 777 " + dirs + "/QT5-C++.sublime-build" )
		os.system( "chmod 777 " + dirs + "/QT5-C++.py" )

def linux_install():
	if not os.path.isdir(linuxInstall):
		os.mkdir( linuxInstall )

	# copia el contenido necesario
	copydir( path + "/bin/linux", linuxInstall + "/bin/linux" )
	copydir( path + "/code", linuxInstall + "/code" )
	copydir( path + "/etc", linuxInstall + "/etc" )
	copydir( path + "/icons", linuxInstall + "/icons" )
	copydir( path + "/log", linuxInstall + "/log" )
	copydir( path + "/os/linux", linuxInstall + "/os/linux" )
	copydir( path + "/sound", linuxInstall + "/sound" )
	copydir( path + "/theme", linuxInstall + "/theme" )
	#-----------------------------------------------------

	#compile_( linuxInstall + "/code/monitor/manager.pro" )
	#compile_( linuxInstall + "/code/manager/monitor.pro" )
	compile_( linuxInstall + "/code/server/server.pro" )

	copyfile( path + "/os/linux/link/CatsFarm.desktop" , "/usr/share/applications")

	shutil.copy( linuxInstall + "/os/linux/init/cserver", "/etc/init.d/cserver")
	os.system( "service cserver start ")

def linux_uninstall():
	
	os.system( "service cserver stop ")
	cserver =  "/etc/init.d/cserver"
	if os.path.isfile(cserver): os.remove( cserver )

	if os.path.isdir(linuxInstall): shutil.rmtree(linuxInstall)
	
	desktop = "/usr/share/applications/CatsFarm.desktop"
	if os.path.isfile(desktop): os.remove(desktop)

def windows_install():
	if not os.path.isdir(windowsInstall):
		os.mkdir( windowsInstall )

	# copia el contenido necesario
	copydir( path + "/bin/win", windowsInstall + "/bin/win" )
	copydir( path + "/code", windowsInstall + "/code" )
	copydir( path + "/etc", windowsInstall + "/etc" )
	copydir( path + "/icons", windowsInstall + "/icons" )
	copydir( path + "/log", windowsInstall + "/log" )
	copydir( path + "/os/win", windowsInstall + "/os/win" )
	copydir( path + "/sound", windowsInstall + "/sound" )
	copydir( path + "/theme", windowsInstall + "/theme" )
	#-----------------------------------------------------
	
	compile_( "CatsFarm-Monitor", windowsInstall + "/code/monitor/main.cpp",  windowsInstall + "/icons/monitor.ico" )
	compile_( "CatsFarm-Manager", windowsInstall + "/code/manager/main.cpp",  windowsInstall + "/icons/manager.ico" )
	compile_( "CatsFarm-Server", windowsInstall + "/code/server/main.cpp",  windowsInstall + "/icons/server.ico" )

	copyfile( windowsInstall + "/os/win/link/CatsFarm Monitor.lnk" , "C:/ProgramData/Microsoft/Windows/Start Menu/Programs")

def windows_uninstall():
	if os.path.isdir( windowsInstall ):
		shutil.rmtree( windowsInstall )
	lnk = "C:/ProgramData/Microsoft/Windows/Start Menu/Programs/CatsFarm Monitor.lnk"
	if os.path.isfile( lnk ):
		os.remove( lnk )

compiler_install()
#sublime_build()

if platform == "win32":
	windows_uninstall()
	windows_install()
elif platform == "linux2":
	None
	linux_uninstall()
	linux_install()




hol a todos