<?php
include_once ("dbconnect.php");

if (isset($_GET["submit"]))
{
    $action = $_GET["submit"];
    if ($action === "clear")
    {
        $sqldelete = "DELETE FROM `tbl_beats`";
        if ($conn->query($sqldelete))
        {
            echo '<script>alert("Cleared")</script>';
            echo '<script>window.location.replace("index.php");</script>';
        }
    }
}
$sqllist = "SELECT * FROM `tbl_beats` ORDER BY beat_date DESC LIMIT 50";
$result = $conn->query($sqllist);
$numofresult = $result->num_rows;
?>

 <!DOCTYPE html>
    <html lang="en">

    <head>
        <meta charset="UTF-8">
        <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <title>BEAT</title>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
        <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
        </style>
    </head>

    <body class="w3-border">
        <header class="w3-center w3-padding-large w3-yellow">
            <h1>Beat</h1>
        </header>
        <div class="w3-bar w3-yellow">
                <div class="w3-bar-item w3-right"><a href="index.php">Refresh</a></div>
                <div class="w3-bar-item w3-right"><a href="index.php?submit=clear" onclick="return confirm('Clear All Data?')">Clear Data</a></div>
            </div>
        </div>
             <?php
if ($result->num_rows > 0)
{
    echo "<p class='w3-center'><b>Number of data ($numofresult FOUND)<br></p></b><hr>";
    echo "<div class='w3-container w3-center' style='overflow-x:auto;min-height:400px; min-width:360px;max-width:600px; margin:auto;display: block; height: 100px;'>";
    echo "<table class='w3-table w3-striped' >
                     <tr><th>No</th><th>ID</th><th>Beat</th><th>Date</th></tr>";
    $labels = array();
    $data = array();
    $i=0;
    while ($row = $result->fetch_assoc())
    {
        $i++;
        $beatid = $row['beat_id'];
        $beat = $row['beat_val'];
        $rdate = date_create($row['beat_date']);
        $date = date_format($rdate, "d/m/Y h:i a");
        $labels[] = $row['beat_id'];
        $data[] = $row['beat_val'];
        echo "<tr><td>$i</td><td>$beatid</td><td>$beat</td><td>$date</td></tr>";

    }
    echo "</table>";
    echo "</div></div>";
    echo "<div class='w3-container' style='height:16px'></div>";
    echo "<hr>";
    echo "<div class='w3-container w3-center' style='min-width:360px;max-width:600px; heigth:400px;margin:auto'><canvas id='myChart'></canvas></div>";
}
else
{
    echo "<p class='w3-center'><b>Number of data ($numofresult FOUND)<br></p></b><hr>";
    echo "<div class='w3-container' style='overflow-x:auto;min-height:400px'>";
}

?>
        </div>
        
             <script>
                // Get the labels and data from PHP and convert them to JavaScript arrays
                var labels = <?php echo json_encode($labels); ?>;
                var data = <?php echo json_encode($data); ?>;
            
                // Create the line chart
                var ctx = document.getElementById("myChart").getContext("2d");
                var myChart = new Chart(ctx, {
                    type: 'line',
                    data: {
                        labels: labels,
                        datasets: [{
                            label: 'Beat',
                            data: data,
                            fill: false,
                            borderColor: 'rgb(75, 192, 192)',
                            tension: 0.1
                        }]
                    },
                    
                    options: {
                        responsive: true,
                        scales: {
                            y: {
                                beginAtZero: true,
                                title: {
                                display: true,
                                text: 'Beat'
                              }
                            },
                            x: {
                                beginAtZero: true,
                                title: {
                                display: true,
                                text: 'ID'
                              }
                            }
                        }
                    }
                });
            </script>
        
    </body>
    </html>
