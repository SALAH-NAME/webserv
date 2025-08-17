<?php
$script_dir = dirname(__FILE__);

$html_file_path = $script_dir . '/php_cgi.html';

try {
    if (file_exists($html_file_path)) {
        $html_content = file_get_contents($html_file_path);
        
        $content_length = strlen($html_content);
        
        echo "Content-Type: text/html\r\n";
        echo "Content-Length: " . $content_length . "\r\n";
        echo "\r\n";
        
        echo $html_content;
    } else {
        $error_content = '<html><body><h1>Error: php_cgi.html not found</h1></body></html>';
        $content_length = strlen($error_content);
        
        echo "Content-Type: text/html\r\n";
        echo "Content-Length: " . $content_length . "\r\n";
        echo "\r\n";
        
        echo $error_content;
    }
} catch (Exception $e) {
    $error_content = '<html><body><h1>Error: ' . htmlspecialchars($e->getMessage()) . '</h1></body></html>';
    $content_length = strlen($error_content);
    
    echo "Content-Type: text/html\r\n";
    echo "Content-Length: " . $content_length . "\r\n";
    echo "\r\n";
    
    echo $error_content;
}
?>
