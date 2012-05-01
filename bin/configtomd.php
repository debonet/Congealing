<?php
 ;



function InputHandler_string($param)
{
	print("<input name='$param[parameter]' type='text' />");
}

function InputHandler_format($param)
{
	print("<input name='$param[parameter]' type='text' />");
}

function InputHandler_integer($param)
{
	if (preg_match('/integer\s*\(([-0-9]*)\s*-\s*([-0-9]*)\)/',$param["type"],$a)){
		print("<input name='$param[parameter]' type='number' min='$a[1]' max='$a[2]' />");
	}
	else{
		print("<input name='$param[parameter]' type='text' />");
	}

}

function InputHandler_real($param)
{
	if (preg_match("/int\s*\[(-0-9.]*)\s*-\s*([-0-9.]*)\s+by\s+([-0-9.]*)\]/",$param["type"],$a)){
		print("<input name='$param[parameter]' type='range' min='$a[1]' max='$a[2]' step='$a[3]'/>");
	}
	else{
		print("<input name='$param[parameter]' type='text' />");
	}
}

function InputHandler_boolean($param)
{
		print("<input name='$param[parameter]' type='radio' value='true'/>True</value>");
		print("<input name='$param[parameter]' type='radio' value='false'>False</value>");
}


function InputHandler_bool($param)
{
		print("<input name='$param[parameter]' type='radio' value='true'/>True</value>");
		print("<input name='$param[parameter]' type='radio' value='false'>False</value>");
}


function InputHandler_enum($param)
{
	preg_match('/\((.*)\)/',$param["type"],$a);

	$as=split(',',$a[1]);

	print("<select name='$param[parameter]'>");
	foreach($as as $s){
		print("<option value='$s'>$s</option>");
	}
	print("</select>");
}



$as=file("config");

foreach($as as $s){
	if (preg_match('/##\s+@([^\s]+)\s+([^\n]*)/i',	$s,	$aM	)){
		$sPart=trim($aM[1]);
		if ($sPart == "parameter"){
			$sParam=trim($aM[2]);
		}
		$aparam[$sParam][$sPart]=trim($aM[2]);
	}
	else if (preg_match('/##\s([^#\n]+)/i',$s,$aM)){
		$aparam[$sParam][$sPart].= "\n ". trim($aM[1]);
	}
}


function CleanSym($s){
	return htmlspecialchars(trim($s));
}

function CleanExample($s){
	return str_replace("\n","\n    ",$s);
};

function CleanLink($s){
	return htmlspecialchars(
		str_replace(
			array("[","]"),
			array("{","}"),
			$s
		)
	);
}

ksort($aparam);


print("# Congealing Parameter Documentation\n\n");

print("## Table of contents\n\n");
foreach($aparam as $param){
	print(
		"1. [" . CleanSym($param['parameter']).  "]"
		. "(#" . CleanLink($param['parameter']) . ")"
		. "\n"
	);
}

foreach($aparam as $param){
	print(
		"\n----------------------------\n"
		.	"<a name='" . CleanLink($param['parameter']) . "' />\n"
		. "### **" 
		. CleanSym($param['parameter']) 
		. "**\n"
	);
	if (array_key_exists('note',$param)){
		print(
			"_" . CleanSym($param['note']) . "_"
			. "\n\n"
		);
	}

	print("#### Type: _" . CleanSym($param['type']) . "_\n\n");
	if (array_key_exists('suggested_range',$param)){
		print(
			"#### Suggested Range: " 
			. CleanSym($param['suggested_range']) 
			. "\n\n"
		);
	}

	print(CleanSym($param['doc']) . "\n\n");
	
//	$sType=preg_replace('/\s.*/',"",$param['type']);
	if (array_key_exists('example',$param)){
		print("#### Example:\n");
		print(CleanExample($param['example']) . "\n\n");
	}


	if (array_key_exists('seealso',$param)){
		print(
			"#### See Also: " 
			. CleanSym($param['seealso']) 
			. "\n\n"
		);
	}
}

?>
