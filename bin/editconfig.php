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
		$sPart=$aM[1];
		if ($sPart == "parameter"){
			$sParam=$aM[2];
		}
		$aparam[$sParam][$sPart]=$aM[2];
	}
	else if (preg_match('/##\s([^#\n]+)/i',$s,$aM)){
		$aparam[$sParam][$sPart].="\n" . $aM[1];
	}
}

?>
<style>
body{
	font-family:Arial,Helvetica;
}

.parameter{
	width:100%;
	margin: 20 0 20 0;
	border: 1 solid #000;
	padding:20px;
}
.parametername{
	font-size:200%;
	text-decoration:underline;
}
.parameterdoc{
	color:#777;
	padding:10 0 10 0;
	font-size:80%;
}

.parametername{
	font-size:200%;
	text-decoration:underline;
}

input[type="text"]{
	width:500
}
input[type="number"]{
	width:70
}

input[type="radio"]{
	width:10
}
</style>

<?php
 ;

foreach($aparam as $param){

	print("<div class='parameter'>");
	print("<div class='parametername'>$param[parameter]</div>");
	print("<div class='parameterdoc'>$param[doc]</div>");
	if (in_array('example',$param)){
		print("<div class='parameterexample'>$param[example]</div>");
	}
	print("<div class='parameterinput'>");
	
	$sType=preg_replace('/\s.*/',"",$param['type']);
	$fxn="InputHandler_$sType";
	$fxn($param);

	print("</div>");
	print("</div>");

}

?>
