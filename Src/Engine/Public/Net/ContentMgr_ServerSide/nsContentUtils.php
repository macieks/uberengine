<?php

function getIntMin($name, $minValue)
{
	$value = $_GET[$name];
	return (is_numeric($value) && $minValue <= $value) ? $value : null;
}

function getIntMax($name, $maxValue)
{
	$value = $_GET[$name];
	return (is_numeric($value) && $value <= $maxValue) ? $value : null;
}

function getIntMinMax($name, $minValue, $maxValue)
{
	$value = $_GET[$name];
	return (is_numeric($value) && $minValue <= $value && $value <= $maxValue) ? $value : null;
}

function beginQuery()
{
	// Validate user

	$password = $_GET['password'];
	$user_name = $_GET['user_name'];

	// TODO
	
	// Get query name
	
	$query = $_GET['query'];
	if (is_null($query))
		return null;
	
	return $query;
}

function endQuery($query_string)
{
	if (is_null($query_string) || strlen($query_string) == 0)
		return;
    
	require('nsContentConfig.php');

	// Connect to data base

	mysql_connect($ue_dbhost, $ue_dblogin, $ue_dbpass) or die(mysql_error());
	mysql_select_db($ue_dbname) or die(mysql_error());

	// Perform query

	$result = mysql_query($query_string) or die(mysql_error());

	// Output query result
  
	// FIXME: Handle strings and binary data properly

	while ($row = mysql_fetch_row($result))
		for ($i = 0; $i < count($row); $i++)
			echo $row[$i]." ";

	echo "<UE_END_OF_QUERY_RESULT>";
}

?>
