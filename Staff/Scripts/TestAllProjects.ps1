
$returnedErrorCodes = New-Object System.Collections.ArrayList
$projectNames = New-Object System.Collections.ArrayList
$pathToRoot = "../../"

#this function allows you to pass the relative path from the root folder of the repo, rather than the relative path from the helper script.
#It depends on pathToRoot being set correctly, but that's only one place to change instead of having to change for each function call, so, more portable. 
function TestProject([string] $PathToCmakeFolder, [string] $PathToPlaceGatware, [string] $ProjectName)
{
	echo "-"
	echo "-"
	echo "-"
	echo "-"
	echo "-[- Making and building $ProjectName -]-"
	& ./HelperScripts/TestProject.bat "$PathToRoot$PathToCmakeFolder", "$PathToRoot$PathToPlaceGatware", $ProjectName
	$projectNames.Add($ProjectName) | Out-Null
	$returnedErrorCodes.Add($LASTEXITCODE) | Out-Null
}

#this function allows you to pass the relative path from the root folder of the repo, rather than the relative path from the helper script.
#It depends on pathToRoot being set correctly, but that's only one place to change instead of having to change for each function call, so, more portable. 
#ProjectName is not used by MakeProject.bat, but it *is* used to track which projects fail.
function MakeProject([string] $PathToCmakeFolder, [string] $ProjectName)
{
	echo "-"
	echo "-"
	echo "-"
	echo "-"
	echo "-[- Making $ProjectName -]-"
	& ./HelperScripts/MakeProject.bat "$PathToRoot$PathToCmakeFolder"
	$returnedErrorCodes.Add($LASTEXITCODE) | Out-Null
	$projectNames.Add($ProjectName) | Out-Null
}

#assignment 1
TestProject "Assignment 1/DirectX11" "Assignment 1/Gateware" "Assignment_1_D3D11"
TestProject "Assignment 1/OpenGL" "Assignment 1/Gateware" "Assignment_1_OpenGL"
TestProject "Variants/Assignment 1/DirectX12" "Variants/Assignment 1/Gateware" "Assignment_1_D3D12"
TestProject "Variants/Assignment 1/Vulkan" "Variants/Assignment 1/Gateware" "Assignment_1_Vulkan"

#assignment 2
TestProject "Assignment 2/DirectX11" "Assignment 2/Gateware" "Assignment_2_D3D11"
TestProject "Assignment 2/OpenGL" "Assignment 2/Gateware" "Assignment_2_OpenGL"
TestProject "Variants/Assignment 2/DirectX12" "Variants/Assignment 2/Gateware" "Assignment_2_D3D12"
TestProject "Variants/Assignment 2/Vulkan" "Variants/Assignment 2/Gateware" "Assignment_2_Vulkan"

#masteries
TestProject "Mastery 1/Vulkan" "Mastery 1/Gateware" "Mastery_1_Vulkan"
TestProject "Mastery 2/Vulkan" "Mastery 2/Gateware" "Mastery_2_Vulkan"

#Debugging Demos
MakeProject "Slides/DebuggingDemos/BrokenStar_DX11" "BrokenStar_DX11"
MakeProject "Slides/DebuggingDemos/BrokenStar_OGL" "BrokenStar_OGL"
MakeProject "Slides/DebuggingDemos/BrokenStar_DX12" "BrokenStar_DX12"
MakeProject "Slides/DebuggingDemos/BrokenStar_VLK" "BrokenStar_VLK"


echo "-"
echo "-"
echo "-"
echo "-"
echo "-[- BUILD RESULTS -]-"
$noErrors = 1
for($i = 0; $i -lt $returnedErrorCodes.Count; $i++) {
	if($returnedErrorCodes[$i] -ne 0 ){
		$failedProject = $projectNames[$i]
		$errorCode = $returnedErrorCodes[$i]
		$position = $i + 1
		$count = $returnedErrorCodes.Count
		echo "- Project $failedProject at position [$position] out of [$count] failed with error code $errorCode  -"
		$noErrors = 0
	}
}
if($noErrors -eq 1)
{
	echo "- No projects failed to build! - "
}

pause
