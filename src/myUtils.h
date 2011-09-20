/*
 CARPE, The Software" © Parag K Mital, parag@pkmital.com
 
 The Software is and remains the property of Parag K Mital
 ("pkmital") The Licensee will ensure that the Copyright Notice set
 out above appears prominently wherever the Software is used.
 
 The Software is distributed under this Licence: 
 
 - on a non-exclusive basis, 
 
 - solely for non-commercial use in the hope that it will be useful, 
 
 - "AS-IS" and in order for the benefit of its educational and research
 purposes, pkmital makes clear that no condition is made or to be
 implied, nor is any representation or warranty given or to be
 implied, as to (i) the quality, accuracy or reliability of the
 Software; (ii) the suitability of the Software for any particular
 use or for use under any specific conditions; and (iii) whether use
 of the Software will infringe third-party rights.
 
 pkmital disclaims: 
 
 - all responsibility for the use which is made of the Software; and
 
 - any liability for the outcomes arising from using the Software.
 
 The Licensee may make public, results or data obtained from, dependent
 on or arising out of the use of the Software provided that any such
 publication includes a prominent statement identifying the Software as
 the source of the results or the data, including the Copyright Notice
 and stating that the Software has been made available for use by the
 Licensee under licence from pkmital and the Licensee provides a copy of
 any such publication to pkmital.
 
 The Licensee agrees to indemnify pkmital and hold them
 harmless from and against any and all claims, damages and liabilities
 asserted by third parties (including claims for negligence) which
 arise directly or indirectly from the use of the Software or any
 derivative of it or the sale of any products based on the
 Software. The Licensee undertakes to make no liability claim against
 any employee, student, agent or appointee of pkmital, in connection 
 with this Licence or the Software.
 
 
 No part of the Software may be reproduced, modified, transmitted or
 transferred in any form or by any means, electronic or mechanical,
 without the express permission of pkmital. pkmital's permission is not
 required if the said reproduction, modification, transmission or
 transference is done without financial return, the conditions of this
 Licence are imposed upon the receiver of the product, and all original
 and amended source code is included in any transmitted product. You
 may be held legally responsible for any copyright infringement that is
 caused or encouraged by your failure to abide by these terms and
 conditions.
 
 You are not permitted under this Licence to use this Software
 commercially. Use for which any financial return is received shall be
 defined as commercial use, and includes (1) integration of all or part
 of the source code or the Software into a product for sale or license
 by or on behalf of Licensee to third parties or (2) use of the
 Software or any derivative of it for research with the final aim of
 developing software products for sale or license to a third party or
 (3) use of the Software or any derivative of it for research with the
 final aim of developing non-software products for sale or license to a
 third party, or (4) use of the Software to provide any service to an
 external organisation for which payment is received. If you are
 interested in using the Software commercially, please contact pkmital to
 negotiate a licence. Contact details are: parag@pkmital.com
 
 
 *
 *
 */

int rgb_colormap[256*3] = {0,0,0,0,0,135,0,0,139,0,0,143,0,0,147,0,0,151,0,0,155,0,0,159,0,0,163,0,0,167,0,0,171,0,0,175,0,0,179,0,0,183,0,0,187,0,0,191,0,0,195,0,0,199,0,0,203,0,0,207,0,0,211,0,0,215,0,0,219,0,0,223,0,0,227,0,0,231,0,0,235,0,0,239,0,0,243,0,0,247,0,0,251,0,0,255,0,4,255,0,8,255,0,12,255,0,16,255,0,20,255,0,24,255,0,28,255,0,32,255,0,36,255,0,40,255,0,44,255,0,48,255,0,52,255,0,56,255,0,60,255,0,64,255,0,68,255,0,72,255,0,76,255,0,80,255,0,84,255,0,88,255,0,92,255,0,96,255,0,100,255,0,104,255,0,108,255,0,112,255,0,116,255,0,120,255,0,124,255,0,128,255,0,131,255,0,135,255,0,139,255,0,143,255,0,147,255,0,151,255,0,155,255,0,159,255,0,163,255,0,167,255,0,171,255,0,175,255,0,179,255,0,183,255,0,187,255,0,191,255,0,195,255,0,199,255,0,203,255,0,207,255,0,211,255,0,215,255,0,219,255,0,223,255,0,227,255,0,231,255,0,235,255,0,239,255,0,243,255,0,247,255,0,251,255,0,255,255,4,255,251,8,255,247,12,255,243,16,255,239,20,255,235,24,255,231,28,255,227,32,255,223,36,255,219,40,255,215,44,255,211,48,255,207,52,255,203,56,255,199,60,255,195,64,255,191,68,255,187,72,255,183,76,255,179,80,255,175,84,255,171,88,255,167,92,255,163,96,255,159,100,255,155,104,255,151,108,255,147,112,255,143,116,255,139,120,255,135,124,255,131,128,255,128,131,255,124,135,255,120,139,255,116,143,255,112,147,255,108,151,255,104,155,255,100,159,255,96,163,255,92,167,255,88,171,255,84,175,255,80,179,255,76,183,255,72,187,255,68,191,255,64,195,255,60,199,255,56,203,255,52,207,255,48,211,255,44,215,255,40,219,255,36,223,255,32,227,255,28,231,255,24,235,255,20,239,255,16,243,255,12,247,255,8,251,255,4,255,255,0,255,251,0,255,247,0,255,243,0,255,239,0,255,235,0,255,231,0,255,227,0,255,223,0,255,219,0,255,215,0,255,211,0,255,207,0,255,203,0,255,199,0,255,195,0,255,191,0,255,187,0,255,183,0,255,179,0,255,175,0,255,171,0,255,167,0,255,163,0,255,159,0,255,155,0,255,151,0,255,147,0,255,143,0,255,139,0,255,135,0,255,131,0,255,128,0,255,124,0,255,120,0,255,116,0,255,112,0,255,108,0,255,104,0,255,100,0,255,96,0,255,92,0,255,88,0,255,84,0,255,80,0,255,76,0,255,72,0,255,68,0,255,64,0,255,60,0,255,56,0,255,52,0,255,48,0,255,44,0,255,40,0,255,36,0,255,32,0,255,28,0,255,24,0,255,20,0,255,16,0,255,12,0,255,8,0,255,4,0,255,0,0,251,0,0,247,0,0,243,0,0,239,0,0,235,0,0,231,0,0,227,0,0,223,0,0,219,0,0,215,0,0,211,0,0,207,0,0,203,0,0,199,0,0,195,0,0,191,0,0,187,0,0,183,0,0,179,0,0,175,0,0,171,0,0,167,0,0,163,0,0,159,0,0,155,0,0,151,0,0,147,0,0,143,0,0,139,0,0,135,0,0,131,0,0,128,0,0	};

#ifdef TARGET_WIN32

#include <windows.h>
#include <WinBase.h>
#include <shlobj.h>
#include <commdlg.h>
#include <shlobj.h>


	WINBASEAPI
	DWORD
	WINAPI
	GetCurrentDirectoryA(
		__in DWORD nBufferLength,
		__out_ecount_part_opt(nBufferLength, return + 1) LPSTR lpBuffer
		);
	WINBASEAPI
	DWORD
	WINAPI
	GetCurrentDirectoryW(
		__in DWORD nBufferLength,
		__out_ecount_part_opt(nBufferLength, return + 1) LPWSTR lpBuffer
		);
	#ifdef UNICODE
		#define GetCurrentDirectory  GetCurrentDirectoryW
	#else
		#define GetCurrentDirectory  GetCurrentDirectoryA
	#endif // !UNICODE


	WINBASEAPI
	BOOL
	WINAPI
	SetCurrentDirectoryA(
		__in LPCSTR lpPathName
		);
	WINBASEAPI
	BOOL
	WINAPI
	SetCurrentDirectoryW(
		__in LPCWSTR lpPathName
		);
	#ifdef UNICODE
		#define SetCurrentDirectory  SetCurrentDirectoryW
	#else
		#define SetCurrentDirectory  SetCurrentDirectoryA
	#endif // !UNICODE

#endif


#ifdef TARGET_WIN32
string getApplicationDirectory()
{
	char originalDirectory[ MAX_PATH ];
	
	GetCurrentDirectoryA( MAX_PATH, originalDirectory );
	return originalDirectory;
}
#endif


string myOpenFolderDialog()
{
	
#ifdef TARGET_WIN32
	//http://msdn.microsoft.com/en-us/library/bb773205(VS.85).aspx
	char szPath[MAX_PATH]; 
	LPMALLOC pMalloc = NULL;
	LPITEMIDLIST pidl = NULL;
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));

	bi.hwndOwner = 0;
	bi.lpszTitle = _T("Select the Folder with the Eye-Tracking Data");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT | BIF_NONEWFOLDERBUTTON | BIF_BROWSEINCLUDEFILES;
	//bi.lpfn = BrowseCallbackProc;
	pidl = SHBrowseForFolder(&bi);
	if(pidl != NULL)
	{
		SHGetPathFromIDListA(pidl, szPath);
	
		if(SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc)
		{
			pMalloc->Free(pidl);  
			pMalloc->Release(); 
			return string(szPath);
		}
	}
#endif
	return string("data/event_data");
}

string myOpenIASFileDialog()
{
	
#ifdef TARGET_WIN32
	//char originalDirectory[ MAX_PATH ];
	//GetCurrentDirectory( MAX_PATH, originalDirectory );
	char szFileName[MAX_PATH] = "";
	
	// http://msdn.microsoft.com/en-us/library/ms646839.aspx
	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = 0;
	ofn.lpstrFilter = "Interest Area Files\0*.ias\0All Files\0*.*\0\0";
	ofn.lpstrFile = szFileName;
	ofn.lpstrTitle = "Open the Interest Area File";
	ofn.nFileOffset = 0;
	ofn.nMaxFile = MAX_PATH;
	ofn.lCustData = 0;
	//ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = 0;
	
	if(GetOpenFileNameA(&ofn)) {
		//loadfile(szFileName);
		//GetCurrentDirectory( MAX_PATH, originalDirectory );
		//SetCurrentDirectory( originalDirectory );
		return string(szFileName);
	}
#endif

	//SetCurrentDirectory( originalDirectory );
	return string("data/event_data/diems01.ias");
}

string myOpenMovieDialog()
{
	
	
#ifdef TARGET_WIN32
	//char originalDirectory[ MAX_PATH ];
	//GetCurrentDirectory( MAX_PATH, originalDirectory );
	char szFileName[MAX_PATH] = "";
	
	// http://msdn.microsoft.com/en-us/library/ms646839.aspx
	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = 0;
	ofn.lpstrFilter = "Movie Files\0*.mov;*.avi;*.xvd\0All Files\0*.*\0\0";
	ofn.lpstrFile = szFileName;
	ofn.lpstrTitle = "Open The Eye-Tracking Movie File";
	ofn.nFileOffset = 0;
	ofn.nMaxFile = MAX_PATH;
	ofn.lCustData = 0;
	//ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = 0;
	
	if(GetOpenFileNameA(&ofn)) {
		//loadfile(szFileName);
		//GetCurrentDirectory( MAX_PATH, originalDirectory );
		//SetCurrentDirectory( originalDirectory );
		return string(szFileName);
	}

	//SetCurrentDirectory( originalDirectory );
	return string("");
#else
	NavDialogCreationOptions dialogOptions;
	OSStatus err = ::NavGetDefaultDialogCreationOptions( &dialogOptions );
	if( err != noErr)
		return	false;
	
	dialogOptions.optionFlags = kNavNoTypePopup + kNavSupportPackages + kNavAllowOpenPackages;
	dialogOptions.modality = kWindowModalityAppModal;
	
	//if(inClientName != NULL)
	//	dialogOptions.preferenceKey = ::CFHash(inClientName);//more or less unique value - enough for this purpose
	
	Boolean outOK = false;
	
	NavDialogRef	dialogRef = NULL;
	
	err = ::NavCreateChooseFileDialog( &dialogOptions,
									  NULL,
									  NULL,
									  NULL,
									  NULL,
									  NULL,
									  &dialogRef);
	
	if( (err == noErr) && (dialogRef != NULL) )
	{
		err = ::NavDialogRun( dialogRef );
		if( err == noErr )
		{
			NavUserAction theAction = ::NavDialogGetUserAction( dialogRef );
			if( (theAction != kNavUserActionCancel) && (theAction != kNavUserActionNone) )
			{
				NavReplyRecord		reply;
				err = ::NavDialogGetReply(dialogRef, &reply);
				if(err == noErr)
				{
					//err = GetFSRefFromNavReply(&reply, outRef);
					::NavDisposeReply( &reply );
					outOK = (err == noErr);
				}
			}
		}
		
		 ::NavDialogDispose( dialogRef );
	}
/*	
	short fRefNumOut;
	FSRef output_file;
	OSStatus err;
	
	NavDialogCreationOptions options;
	NavGetDefaultDialogCreationOptions( &options );
	options.modality = kWindowModalityAppModal;
	//options.windowTitle = CFSTR(msg.c_str());
	options.message = CFSTR("Open Eye-tracking Movie File...");
	
	
	NavDialogRef dialog;
	err = NavCreateChooseFileDialog(&options, '.mov', NULL, NULL, NULL, NULL, &dialog);
	//err = NavCreatePutFileDialog(&options, '.mov', 'Moov', NULL, NULL, &dialog);
	printf("NavCreateChooseFileDialog returned %i\n", err );
	
	err = NavDialogRun(dialog);
	printf("NavDialogRun returned %i\n", err );
	
	NavUserAction action;
	action = NavDialogGetUserAction( dialog );
	printf("got action %i\n", action);
	if (action == kNavUserActionNone || action == kNavUserActionCancel) {
		
		printf("Encountered Error During Get User Action");
	}
	
	// get dialog reply
	NavReplyRecord reply;
	err = NavDialogGetReply(dialog, &reply);
	if ( err != noErr )	{
		
		printf("Encountered Error During Reply");
	}
	if ( reply.replacing )
	{
		printf("Need to replace\n");
	}
	
	AEKeyword keyword;
	DescType actual_type;
	Size actual_size;
	FSRef output_dir;
	err = AEGetNthPtr(&(reply.selection), 1, typeFSRef, &keyword, &actual_type,
					  &output_dir, sizeof(output_file), &actual_size);
	
	//printf("AEGetNthPtr returned %i\n", err );
	
	
	CFURLRef cfUrl = CFURLCreateFromFSRef( kCFAllocatorDefault, &output_dir );
	CFStringRef cfString = NULL;
	if ( cfUrl != NULL )
	{
		cfString = CFURLCopyFileSystemPath( cfUrl, kCFURLPOSIXPathStyle );
		CFRelease( cfUrl );
	}
	
    // copy from a CFString into a local c string (http://www.carbondev.com/site/?page=CStrings+)
	const int kBufferSize = 255;
	
	char folderURL[kBufferSize];
	Boolean bool1 = CFStringGetCString(cfString,folderURL,kBufferSize,kCFStringEncodingMacRoman);
	
	char fileName[kBufferSize];
	Boolean bool2 = CFStringGetCString(reply.saveFileName,fileName,kBufferSize,kCFStringEncodingMacRoman);
	
	// append strings together
	
	string url1 = folderURL;
	string url2 = fileName;
	string finalURL = url1 + "/" + url2;
	
	printf("url %s\n", finalURL.c_str());
	
	// cleanup dialog
	NavDialogDispose(dialog);
	//	saver.setup(mov.width,mov.height,finalURL.c_str());
	return finalURL;
	 */
	
#endif
	
#ifdef TARGET_WIN32
	//	saver.setup(mov.width,mov.height,"output.mov");
#endif
return false;

}
/*
string myDialog()
{
	static DWORD nFilterIndex=0;
	CFileDialog dlg(TRUE,NULL,NULL,OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_DONTADDTORECENT | OFN_NOCHANGEDIR, NULL);

	char sCurDir[_MAX_PATH];
	::GetCurrentDirectory(_MAX_PATH,sCurDir);
	dlg.m_ofn.lpstrTitle=IDS_H_KULISSE;
	dlg.m_ofn.lpstrInitialDir=theApp.m_pfadKulissen;
	dlg.m_ofn.nFilterIndex=nFilterIndex;
	if(dlg.DoModal()==IDOK)
	{

	}
	::SetCurrentDirectory(sCurDir);
}*/

void find_and_replace( string &source, const string find, string replace ) 
{
	size_t j;
	for ( ; (j = source.find( find )) != string::npos ; ) 
	{
		source.replace( j, find.length(), replace );
	}
}
   
void jetColorMap(unsigned char *rgb,float value,float min,float max)
{
	float max4=(max-min)/4;
	int c1 = 0;
	value-=min;
	if(value==HUGE_VAL)
    {rgb[0]=rgb[1]=rgb[2]=255;}
	else if(value<=0)
    {rgb[0]=rgb[1]=rgb[2]=0;}
	else if(value<max4)
    {rgb[0]=0;rgb[1]=0;rgb[2]=c1+(unsigned char)((255-c1)*value/max4);}
	else if(value<2*max4)
    {rgb[0]=0;rgb[1]=(unsigned char)(255*(value-max4)/max4);rgb[2]=255;}
	else if(value<3*max4)
    {rgb[0]=(unsigned char)(255*(value-2*max4)/max4);rgb[1]=255;rgb[2]=255-rgb[0];}
	else if(value<max)
    {rgb[0]=255;rgb[1]=(unsigned char)(255-255*(value-3*max4)/max4);rgb[2]=0;}
	else {rgb[0]=255;rgb[1]=rgb[2]=0;}
}

// from http://nehe.gamedev.net/tutorial/vertex_buffer_objects/22002/
bool IsExtensionSupported( char* szTargetExtension )
{
    const unsigned char *pszExtensions = NULL;
    const unsigned char *pszStart;
    unsigned char *pszWhere, *pszTerminator;
 
    // Extension names should not have spaces
    pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
    if( pszWhere || *szTargetExtension == '\0' )
        return false;
 
    // Get Extensions String
    pszExtensions = glGetString( GL_EXTENSIONS );
 
    // Search The Extensions String For An Exact Copy
    pszStart = pszExtensions;
    for(;;)
    {
        pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
        if( !pszWhere )
            break;
        pszTerminator = pszWhere + strlen( szTargetExtension );
        if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
            if( *pszTerminator == ' ' || *pszTerminator == '\0' )
                return true;
        pszStart = pszTerminator;
    }
    return false;
}