University of California Davis Center for Water-Energy Efficiency Energy Demand Management Software for Water Distribution Utilities.

# WaterWatch Energy Demand Management Software for Water Distribution Systems
Developed by the University of California Davis, Center for Water-Energy Efficiency, Project CECFL79.     
Funded by the California Energy Commission EPIC program, Award EPC-16-062.        
> Principle Investigator: Dr. Frank Loge, PE.       
> Engineering Manager and Design Lead: Robert Good.     
> PhD Student Researcher: Erin Musabandesu, PE.    
> PhD Student Researcher: Greg Miller.    
> Research Engineer: David Linz.    
> Research Engineer: Behzad Ahmadi.     
> UWP Graphical User Interface Developer: Allen Huber.      

## Purpose
The water utility sector currently has few options for controlling their energy demand during specific time periods - including reducing, shifting, or ramping up loads to adhere to different energy rate structures. Typically, water distribution systems are managed by meeting customer water demands which fluctuate throughout the day and over the course of a week. To ensure customer needs are met, water system operators focus on meeting demands. Optimizing energy use to save costs is often an afterthought (or simply unmanageable). There is a gap in knowledge of how to effectively manage the energy demands of tens to hundreds of pumps within a distribution system, while still ensuring customer water demands are sufficiently met. In addition, evaluating energy use and costs under different energy rate structures and changing operational practices to reduce, shift, or increase demands during specific time periods is challenging and requires continuous data analysis. Due to the complexity of energy demand management in the water utility sector and the lack of knowledge in the form of effective market solutions, water utilities are often unable to participate in demand response events, or shift energy loads outside peak hours to reduce impact to the grid and reduce energy costs.  

This problem has not yet been addressed widespread because the solution requires accessing real-time water and energy data and an analysis engine to define and quantify reduction opportunities. 

The Center for Water-Energy Efficiency (CWEE) at the University of California, Davis has combined water system hydraulic modeling with operations analytics and optimization algorithms to produce a DMS that generates the logic and controls for a water system. 

## System Requirements
Minimum requirements:
> OS:             Windows 10.0.17134 Build 17134        
> CPU:            4 logical cores, 2 GHz Intel compatible.      
> System Memory:  8192MB+       
> Graphics card:  N/A       

Recommended requirements:
> OS:             Windows 10.0.18362 Build 18362        
> CPU:            8 logical cores, 3 GHz Intel compatible.      
> System Memory:  16384MB+
> Graphics card:  N/A       

## Other Requirements
Software: Visual Studio 2019 ([Install the free Visual Studio Community 2019 here](https://visualstudio.microsoft.com/downloads/))

Github account ([Sign up for Github here](https://github.com/join))

## Code Check Out and Set Up in Visual Studio 2019 (Required For All Versions)
1. On the EDMS project page in Github, click the green “Clone or download” dropdown button at the top of the page and copy the URL (or just directly copy this URL: https://github.com/ucd-cwee/EDMS.git)

2. Open Visual Studio and select “Clone or Check Out Code.” 

3. Paste the URL into the “Repository Location” field and select the local path where you would like the files to download. Click “Clone”.

4. If you have not already done so already, log into your GitHub account when Visual Studio prompts you to.

5. Once the repository is loaded, click the "Switch Views" button at the top of the Solution Explorer window. Select "EDMS.sln".

6. At this point Visual Studio will notify you if you are missing any necessary packages to run the software. Click install and wait for the installer to finish. You will be required to restart your computer after these are installed. 

7. Once your computer restarts, open Visual Studio Installer and select to "modify" the version of Visual Studio you will utilize for this software. Under individual components, please install the following extensions: 
   > SQL Server Data Tools                      
   > C++ ATL for latest vXXX build tools (x86 & x64)                             
   > C++ ATL for latest vXXX build tools with Spectre Mitigations (x86 & x64)                      
   > C++ MFC for latest vXXX build tools (x86 & x64)                             
   > C++ MFC for latest vXXX build tools with Spectre Mitigations (x86 & x64)                      

8. Once all installations are complete, open Visual Studio and open the project by selecting the "EDMS.sln" file in the EDMS repository.

## Compiling and Running the Windows 10 UWP Application with Visual Studio 2019 Community or Professional Edition
1. After opening the "EDMS.sln" file in Visual Studio, from the toolbar select "Build", then "Configuration Manager". Set the Active solution configuration to "Release", and the Active solution platform to "x64". 

2. From the Solution Explorer window, right-click “EDMS_UWP_App (Universal Windows)” and select "Set as StartUp Project". 

3. In order to run the application, you will either need to install the cwee_cert.pfx certificate file (provided by CWEE) on your computer, separate from Visual Studio, or self-certify. To implement self-certification, please follow the instructions provided by Microsoft here: ([Create A Self-Signed Certificate](https://docs.microsoft.com/en-us/windows/msix/package/create-certificate-package-signing#create-a-self-signed-certificate))

4. From the Solution Explorer window, right-click “EDMS_UWP_App (Universal Windows)” and select the wrench icon to open the Properties page. Select the "Application" page in the left menu. Click "Package Manifest...", then "Packaging". You can change the Package Name if desired. Select "Choose Certificate...". Select the certificate previously installed. 

5. Rebuild the entire solution (From the Solution Explorer window, right-click "Solution 'EDMS'" and select "Rebuild Solution").

7. Deploy the UWP application (From the Solution Explorer window, right-click "EDMS_UWP_App (Universal Windows)" and select "Deploy").
 
8. Rebuild the UWP application (From the Solution Explorer window, right-click "EDMS_UWP_App" and select "Rebuild"). This will perform several pre- and post-build events which copy required DLLs into the application's root directory, that had been removed after 'Deploy' was performed.
 
9. Run the application (From the toolbar select "Debug", then "Start Debugging").

10. Once the app opens, right click on the app icon in your taskbar and select "Pin to taskbar" to be able to open the app in the future. You can also open the app by searching "EDMS" in the start menu.

11. The local project files will be saved to a hidden directory on the computer. Typical path:                    
 "C:\Users\ & username & \AppData\Local\Packages\ & Package Name & \LocalState\EDMS_Files\"

## Compiling and Running the Win32 Console Application on Windows 10 with Visual Studio 2019 Community or Professional Edition
1. After opening the "EDMS.sln" file in Visual Studio, from the toolbar select "Build", then "Configuration Manager". Set the Active solution configuration to "Release", and the Active solution platform to "x64". 

2. From the Solution Explorer window, right-click “EDMS_Wind32_Console” and select "Set as StartUp Project". 

3. Rebuild the entire solution (From the Solution Explorer window, right-click "Solution 'EDMS'" and select "Rebuild Solution").

4. Run the application (From the toolbar select "Debug", then "Start Debugging").

5. The local project files will be saved to a directory downstream of the .exe file. Typical path:                     
 "C:\Users\ & username & \source\repos\EDMS\x64\Release\data\"

## License
 Some of the source code utilized and compiled by this software includes modified source code from public, 
 free software under the terms of the GNU General Public License (GPL). This software adopts the terms of the 
 GNU GPL as published by the Free Software Foundation, either version 3 of the License, or (at your option) 
 any later version. 

 See COPYING.txt for the GNU General Public License including certain additional terms described below and 
 contained in this source code. 

### zLib Library
> cweeLib/Sys/Unzip/*     
> Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler
>
> This software is provided 'as-is', without any express or implied
> warranty.  In no event will the authors be held liable for any damages
> arising from the use of this software.
>
> Permission is granted to anyone to use this software for any purpose,
> including commercial applications, and to alter it and redistribute it
> freely, subject to the following restrictions:        
> 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.       
> 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.     
> 3. This notice may not be removed or altered from any source distribution.        

### OpenStreetMap Geocoding and Reverse Geocoding using cNominatim
> cweeLib/Sys/GeoCoding
> Copyright (c) 2010 - 2020 by PJ Naughter (Web: www.naughter.com, Email: pjna@naughter.com)
>
> All rights reserved.
> 
> Copyright / Usage Details:
>
> You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
> when your product is released in binary form. You are allowed to modify the source code in any way you want 
> except you cannot modify the copyright details at the top of each module. If you want to distribute source 
> code with your application, then you are only allowed to distribute versions released by the author. This is 
> to maintain a single distribution point for the source code.

### Efficient Random Access Hash Map using Tessil
> cweeLib/Utilities/Lists/tsl
> MIT License
> 
> Copyright (c) 2017 Tessil
> 
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.

### Hydraulic Network Simulations using EPAnet
> EDMS_DLL_Codebase/Backend/EPAnet
> MIT License
> 
> Copyright (c) 2017 Open Water Analytics
> 
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.

### SQL Database Connectivity Using nanodbc
> cweeLib/Sys/ODBC
> Copyright (C) 2013 lexicalunit <lexicalunit@lexicalunit.com>
> 
> The MIT License
> 
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in
> all copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
> THE SOFTWARE.

### Online Data Retrieval using CURL 
> EDMS_DLL_Codebase/Backend/Remote_Data
> Copyright (C) 1998 - 2019, Daniel Stenberg, <daniel@haxx.se>, et al.
> 
> This software is licensed as described in the file COPYING, which
> you should have received as part of this distribution. The terms
> are also available at https://curl.haxx.se/docs/copyright.html.
> 
> You may opt to use, copy, modify, merge, publish, distribute and/or sell
> copies of the Software, and permit persons to whom the Software is
> furnished to do so, under the terms of the COPYING file.
> 
> This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
> KIND, either express or implied.

### IO for Uncompress .zip Files using zLib
> cweeLib/Sys/Unzip/*       
> Copyright (C) 1998 Gilles Vollant         
> zlib is Copyright (C) 1995-1998 Jean-loup Gailly and Mark Adler
> 
> This software is provided 'as-is', without any express or implied
> warranty.  In no event will the authors be held liable for any damages
> arising from the use of this software.
> 
> Permission is granted to anyone to use this software for any purpose,
> including commercial applications, and to alter it and redistribute it
> freely, subject to the following restrictions:        
> 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.       
> 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.     
> 3. This notice may not be removed or altered from any source distribution.        

### IO for Compressing .zip Files using Minizip and zLib
> cweeLib/Sys/File/ZipSys/*       
> MIT License     
> Copyright (c) 2017 Sygmei
> 
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.

### Math, Memory, Multithreading, and Strings 
> cweeLib/Math/*, cweeLib/Sys/*, cweeLib/Mem/*, cweeLib/Strings/*     
> Copyright (C) 1993-2012 id Software LLC, a ZeniMax Media company.       
> Doom 3 BFG Edition GPL Source Code is Copyright (C) 1993-2012 id Software LLC
> 
> This software is provided 'as-is', without any express or implied
> warranty.  In no event will the authors be held liable for any damages
> arising from the use of this software.
> 
> Permission is granted to anyone to use this software for any purpose,
> including commercial applications, and to alter it and redistribute it
> freely, subject to the following restrictions:
> 
> 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.       
> 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.     
> 3. This notice may not be removed or altered from any source distribution.
 
### Machine Learning
> cweeLib/math/MachineLearning/*      
> Copyright (C) 2006  Davis E. King (davis@dlib.net)      
> Dlib Modern C++ Machine Learnign Toolkit License uses the Boost         
> Software License - Version 1.0 - August 17th, 2003
> 
> Permission is hereby granted, free of charge, to any person or organization
> obtaining a copy of the software and accompanying documentation covered by
> this license (the "Software") to use, reproduce, display, distribute,
> execute, and transmit the Software, and to prepare derivative works of the
> Software, and to permit third-parties to whom the Software is furnished to
> do so, all subject to the following:
>   
> The copyright notices in the Software and this entire statement, including
> the above license grant, this restriction and the following disclaimer,
> must be included in all copies of the Software, in whole or in part, and
> all derivative works of the Software, unless such copies or derivative
> works are solely in the form of machine-executable object code generated by
> a source language processor.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
> SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
> FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
> ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
> DEALINGS IN THE SOFTWARE.

## Known Issues
 None at this time. 

## Bug Reporting
 Please email Robert Good at (rtgood@ucdavis.edu) to report bugs and crashes. Please specify how to reproduce the bug or crash if at all possible. 

## Ackowledgements
 Special thanks to the California Energy Commission for financing this study and development with funds from the Electric Program Investment Charge (EPIC), the ratepayer surcharge authorized by the California Public Utilities Commission.        
 
 Special thanks to the electricity ratepayers in California for funding this and other research studies.            
 
 Special thanks to Dr. Frank Loge for his continued mentorship and input throughout design and development.           
 
 Special thanks to the Technical Advisory Committee for their guidance, recommendations, and assistance throughout the project.   
 
 We appreciate the support of Kendra Olmos, Halona Leung, Carolyn Forlee, and Camille Agnew throughout the project to administer office and contract responsibilities.    
 
 We appreciate the diligence of the Moulton Niguel Water District management and staff in supporting this project, piloting this software, and generously supporting the research community.   
 
 We are grateful to our graduate student researchers who have poured their hearts into this software and demonstrated extreme understanding of water system operations.      
 
 We are grateful for the companionship of our peers; Amanda Rupiper, Richard Lee, Soraya Manzor, Jon Martindill, Philip Voris, and all others at the Center and the Energy and Efficiency Institute.   
 
 We are eternally grateful to our wives, husbands, and partners for their support and encouragement through the many, long months of development.           
 