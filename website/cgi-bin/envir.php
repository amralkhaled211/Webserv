#!/usr/bin/env php
<?php

// Output the HTTP headers
header("Content-Type: text/html");

// Generate the HTML content
echo "<!DOCTYPE html>";
echo "<html>";
echo "<head><title>Environment Variables</title></head>";
echo "<body>";
echo "<h1>Environment Variables</h1>";
echo "<ul>";

// Iterate over the environment variables and print them
foreach ($_SERVER as $key => $value) {
    echo "<li><strong>$key:</strong> ";
    if (is_array($value)) {
        echo "<pre>" . print_r($value, true) . "</pre>";
    } else {
        echo htmlspecialchars($value);
    }
    echo "</li>";
}

echo "</ul>";
echo "</body>";
echo "</html>";
?>