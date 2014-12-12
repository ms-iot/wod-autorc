The Self Driving RC Car
=======================

So, you want to make a self driving car. Ambitious. So are we. So we built one. Only, it was a little bit smaller than the real thing. 

You can build one too! In fact, we challenge you to do so. Here on this website, we'll tell you how we built ours, and hopefully that will give you a starting point for your own creation -- something faster, smarter, and cooler. 

When you've built it, share your design with us, along with a video of it working. The best projects will be rewarded for their awesomeness with cool Windows on Devices dev kits, shoutouts, and more!

But lets get right into it. This is how we built our car.

##Contents
- [The Supplies](#The-Supplies)
- [The Smarts (and the Software)](#The-Smarts-(and-the-Software))
- [Conclusion](#Conclusion)


##The Supplies

We started with a car. Specifically a toy car. More specifically, a replica red (everyone knows red cars go faster) [Ford Mustang 1967 by Maisto Tech](http://www.amazon.com/Maisto-1967-Mustang-Colors-Styles/dp/B000FJEM14), complete with a racing stripe (and everyone knows cars with racing stripes are faster).

Next, we needed something to act as the brains of our car. Here at Microsoft, we've been working on [Windows on Devices](http://www.windowsondevices.com), a completely free, completely awesome, version of Windows that can run on devices (of which, we chose to use an [Intel Galileo Gen2](http://www.intel.com/content/www/us/en/do-it-yourself/galileo-maker-quark-board.html)).

Windows on Devices has all the goodness that you get when developing for Windows: Networking, File System support, plug and play USB, to name a few. But you get so much more: GPIO, PWM, SPI, and Arduino Compatibility. Familiar with Arduino? Great! You can write and copy your exisiting Sketch code on Windows, and it will just work!

Ok, we've got the car and the brains. We need a few more pieces:

####The eyes:

Since a car can't drive itself very well without knowing about the world around it, we decided to use 4 [Infared Proximity Sensors](https://www.sparkfun.com/products/242). These sensors deliver an analog output that is proportional to how close they are to an object, and have a range of 80cm. 

Secondly, most RC car toy DC motors don't have any kind of feedback, so it would be useful to know things like whether or not the car is moving, and how fast it's going. To accomplish these, we mounted a [Photoresistor](https://www.sparkfun.com/products/9088) in front of one of the forward wheels, which we painted half-white. We'll get into this a bit later.

####The motors:

The first thing we're going to do when we start building our self-driving car is to remove the existing internals that come with the RC Car. This leaves us without a motor driver to power the motors, so we suggest wiring up an L298N. We used an [L298N breakout board](http://www.amazon.com/SainSmart-Stepper-Controller-Mega2560-Duemilanove/dp/B00AJGM37I/ref=sr_1_3?ie=UTF8&qid=1417733989&sr=8-3&keywords=L298N).

##The Assembly

You've got what you need? Good. Let's put it together.

First, carefully take apart the RC Car (try not to lose all those screws, it makes putting the car back together again a bit difficult). When the top chassis is removed, carefully cut the leads to the circuit board inside the RC Car. Make note of which wires go to the battery positive and ground and which go to which motors. You may want to label them.

Next, connect the positive and negative leads from the battery to Vin and GND of the Motor Driver board respectively. Connect the rear motor to OUT3 and OUT4 of the motor driver, and the front motor to OUT1 and OUT2. 

After that, connect GND of the Galileo to GND of the Motor Driver board. 

Now, we want to set up the pins that will logically control our motors. Wire pins 5 and 6 from the Galileo to pins IN1 and IN2 of the Motor Driver board, and pins 11 and 12 of the Galileo to pins IN3 and IN4 of the Motor Driver board.

The Motor Driver board is powered by an L298N H-Bridge chip. By driving one input high and another low for each pair, you dictate the direction that current flows accross the respective motor. This, in turn, dictates the direction the motor turns in. 

We've chosen pins 5, 6, 11, and 12 as our driving pins because they are PWM enabled. This allows us to drive the motors at varying speeds. 

Next, we'll wire up our sensors. Each infared sensor has three leads, one yellow, one red, and one black. Wire the black wires to GND, the red to +5V.

Mount a sensor on the front of the car, angled 45 degrees left. Wire the yellow line from the sensor to A0 on the Galileo. 

Mount another sensor centered at the front, and wire that yellow wire to A1 on the Galileo.

Mount a third sensor angled 45 degrees right, sitting at the front of the car. Wire the yellow wire to A2 on the Galileo.

Finally, mount the fourth sensor centered at the rear of the car, and wire the yellow wire to A4.

These sensors will help us steer the car and avoid walls and obstacles. 

But we won't be able to avoid everything, since our setup doesn't have sensors to watch all 360 degrees around it. To fix this, we will mount a photoresistor to examine the state of one of the front wheels. First, you'll want to either paint half the wheel white, or tape a white piece of paper on half of the wheel. Now, mount the photoresistor on the bottom bumper so that it's facing the wheel. Secure the photoresistor so it doesn't move around if the car were to bump into stuff.

You'll want to connect one end of the photoresistor to both a 10kOhm resistor and A3 on the Galileo. The other end of the photoresistor should be wired directly to ground. Finally, wire the unconnected end of the 10kOhm resistor to +5V. 

If you're unclear about any of the instructions above, check out the Fritzing diagram below. 

![Windows On Devices - AutoRC Wiring](/images/AutoRC_bb.png "Windows On Devices - AutoRC Wiring")

Once you've got this all wired up, we're ready to start programming the brain behind our car!

##The Smarts (and the Software)

The software in our Autonomous RC Car will have a couple different components:

1. Front / Back Control
  This will be the most complicated part of our software. Here, we'll implement a simple PID controller (never heard of one before? You'll be able to read more about it down below!). The PID controller will drive the car, trying to find an optimal distance from the wall. At which point, we'll reverse the PID controller to use the rear sensor, and turn in random directions. 

2. Left / Right Control
  This will compare the values of the left and right sensors to determine when turning is neccessary. It'll be pretty straightforward, only consiting of a couple conditionals.

3. Speed and Stuck Detection
  This will use the photoresistor value to determine whether our car is stuck. We'll also talk about how you might use this reading to determine the speed of your car.

But before we talk about that, we need to cover some housekeeping.

###Housekeeping
In Visual Studio, create a new Galileo Sketch Project. First, we define constants for each of the sensors and their pins 

	//define input pins
	#define LEFT_SENSOR 2
	#define RIGHT_SENSOR 0
	#define FWD_SENSOR 1
	#define WHEEL_SENSOR 3
	#define REAR_SENSOR 4

	//define output pins
	#define OUT_RIGHT = 11;
	#define OUT_UP = 6;
	#define OUT_LEFT = 10;
	#define OUT_DOWN = 5;

We'll also define an enum to represent whether we are in forward or backwards driving mode, a buffer of ints to store the historical values of the wheel readings, and an array to store the id's of the sensors we're reading from and to place the values in. We'll also define a boolean where we keep track of whether or not the car is stuck.

	enum DRIVING_STATE { FORWARD, BACKWARD };
	DRIVING_STATE m_state;
	bool m_stopped = false;
	
	//statics in which to store readings from sensors
	const int SENSORS[] = { A0, A1, A2, A3, A4 };	
	int READINGS[] = { 0, 0, 0, 0, 0 };
	int WHEEL_READINGS[1000];

Next, lets take a look at what our sketch's loop() function will look like:

	//Loop Arduino function
	void loop()
	{
		//read in sensor values
		for (int i = 0; i < 5; i++)
		{
			READINGS[i] = analogRead(SENSORS[i]);
		}
	
		//perform median filtering on readings
		medianFilter();
		
		stuckDetection();
		drive();
	}

Our main logic loop is simple: read our sensors, run the values through a median filter to make sure we throw out noise and outliers, and then drive the car appropriately (depending on whether we are stuck or not).

Before moving on, we'll take a quick look at how the simple median filter works.
```cpp
void medianFilter()
{
	static const size_t FRAME_SIZE = 20;
	static int data[SENSOR_COUNT][FRAME_SIZE];
	static int sorted_data[SENSOR_COUNT][FRAME_SIZE];
	static size_t data_size = 0;

	//shift data
	data_size = min(FRAME_SIZE, data_size + 1);
	for (int n = 0; n < SENSOR_COUNT; n++)
	{
		for (int i = data_size - 1; i > 0; i--) {
			data[n][i] = data[n][i - 1];
		}
	}

	//add in new value
	for (int n = 0; n < SENSOR_COUNT; n++)
	{
		data[n][0] = READINGS[n];
	}

	// Create a sorted array
	for (int n = 0; n < SENSOR_COUNT; n++)
	{
		memcpy(sorted_data[n], data[n], data_size * sizeof(int));
		std::qsort(sorted_data[n], data_size, sizeof(int), compare_ints);
	}

	// Get median
	for (int n = 0; n < SENSOR_COUNT; n++)
	{
		READINGS[n] = sorted_data[n][data_size / 2];
 	}
}
```
First, we define a frame size for each variable. Here, we use a frame size of 20 for each reading. That means the median filter will return the median value out of the last 20 readings.  It does this by sorting the list of values and returning the one in the middle. This is a way of averaging out readings, and should eliminate a lot of sensor noise and spikes, allowing for smoother control.

If you're having trouble wrapping your head around what the median filter does, take a look at the following two graphs. The first is a graph of a line ( y = x ) multiplied with a random value between 0.85 and 1.5. You can see a lot of spikes in the graph. The second graph is the same data, passed through a median filter with a frame size of 10. As you can see, the second graph still maintains the general response, but filters out many of the spikes (which if used as data to drive a controller would result in weird behavior).

![Raw vs Median Filtered Data](/images/raw-vs-filtered.png "Raw vs Median Filtered Data")

If you want to play around with this data, you can see the JavaScript snippet that we used to generate the example data, available in example-snippets/.

###1. Front/Back Control (PID)

PID controllers (Proportional-Integral-Derivative), if you've never heard of them, are some of the most commonly implemented controllers in existence. PID Controllers can be found in the guts of nuclear reactors, heating and cooling systems, toys, robots, military helicopters and jets, and so much more. 

For the following section, we'll use our Auto-RC Car as the basis for our discussion. We'll assume that our sensor value is the forward reading of the infared sensor, and we'll say that in the ideal case, a reading of 300 correlates to an acceptable distance from the wall.

In essence, PID controllers work by looking sensor readings and comparing them to an ideal value. In this case, our ideal value is 300. This error value is then used to determine how to control the device in the following way:

1. Proportional
  This part works by looking at how far the sensor value is from the ideal value right now. If the sensor is reading 400, and we want it to read 300, that means our error is 100. The speed at which we drive the motors would then be dictated by this error rate.
2. Integral
  This part looks at the history of the error values. For example, if our sensor value is changing slowly, the car might start to speed up to compensate for the fact that our error isn't being fixed fast enough.
3. Derivative
  This part looks at how the error is changing. Imagine that our sensor values are approaching our ideal value faster and faster. If this is happening too fast, we might hit a wall! The derivative term kicks in when the error changes, and it's effect is proportional to the rate of that change.

You can take a look at our PID implementation in PID.cpp and PID.h. This is heavily inspired by Bret Beauregard's work with the Arduino PID library. You should check out [his blog](http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/) on the topic to learn more about how it works.

An important part of implementing a PID controller is the process of turning parameters. In other words, deciding how big of a factor the Proportional, Integral, and Derivative portions will play moving forward. 

[Wikipedia](http://en.wikipedia.org/wiki/PID_controller) has a good table which discusses the effect of each parameter. You'll have to figure out good values to use based on the flooring of your home, the torque of your motors, and the changing power output of your battery. Below are three charts which compare the response of the car at different parameter settings. The "Output" value indicates how strong the motor is being driven - with 255 being max forward and -255 being max backward. The error indicates how far the car is away from the ideal point away from the wall, based on how we implemented this, that's about 30cm away.

The proportional component is the first big parameter to tune. This has a big impact on how fast your car will move and respond to changes. 

![Varying The Proportional Term](/images/kp-varies.png "Varying The Proportional Term")

You can see that when the proportional input is higher (the blue line) we get a faster response, and a final value that has less error. The downside is we get more oscillations -- when the car gets close to the goal, it'll bounce back and forth for a while before settling down. 

Personally, we wanted our car to drive smooth, even if that meant it was a bit slower. It didn't really matter to us how far it stopped from the wall, only that it stopped before hitting it. Because of this, we decided to go with a smaller proprtional factor. You might decide to use something different. 

Second, we tuned the integral parameter. This parameter helps to reduce the final error, and speeds up the response.

![Varying The Integral Term](/images/ki-varies.png "Varying the Integral Term") 

You might notice we picked very small values for this parameter. There's a reason for that. Because most of the time, the car is ideally powering down a hallway, having a high integral term would mean that it keeps going faster and faster and faster, and would eventually be very hard to correct. In order to deal with this issue, we used both a small integral term, and we placed a hard limit to how much the integral term could affect the speed of the car (check out PID.cpp).

Finally, we played with the derivative term. This term dictates how quickly the controller reacts to change. 

![Varying The Derivative Term](/images/kd-varies.png "Varying The Derivative Term")

You might notice that the blue line gets to the steady-state (i.e., non-changing error) slightly faster than the orange line (which has a smaller derivative term). Depending on your setup, you might not see a huge effect from the derivative term, and it's one of the hardest terms to tune by hand, so we decided to keep it fairly small.

Here's the response of the car, based on the final parameters we chose.

![Final Car Behaviour](/images/final.png "Final Car Behaviour")

At the end of the day, parameter tuning is a very challenging problem, especially when batteries start to drain, or when conditions change. We'd love to see what solutions you come up with -- whether it be an algorithm that auto-tunes your parameters, or a car that can detect conditions and use different parameter presets automatically! 

###2. Left/Right Control

Steering is pretty straightforward. We compare the value of one of the L/R sensors with that of the other. If one of them is big enough, and there's enough clearance to turn (i.e., one of them is small, the other is big), and the car is actually moving (no point turning the wheels if we're stopped), then we turn in the appropriate direction.

There's also a check to see if we're moving forwards or backwards, to turn appropriately. The snippets that handle this are in Main.cpp, but are reproduced below for easy reference:
```cpp
if (state == BACKWARD)
{
	if (abs(READINGS[RIGHT_SENSOR] - READINGS[LEFT_SENSOR]) < TURN_THRESHOLD)
	{
		turnStraight();
	}
	else if (READINGS[LEFT_SENSOR] > TURN_BOUNDARY
		&& READINGS[RIGHT_SENSOR] < (READINGS[LEFT_SENSOR] - TURN_THRESHOLD)
		&& abs(controller.state.output) > THROTTLE_THRESHOLD)
	{
		turnLeft();
	}
	else if (READINGS[RIGHT_SENSOR] > TURN_BOUNDARY
		&& READINGS[LEFT_SENSOR] < (READINGS[RIGHT_SENSOR] - TURN_THRESHOLD)
		&& abs(controller.state.output) > THROTTLE_THRESHOLD)
	{
		turnRight();
	}
	else
	{

		if (direction == 0)
		{
			turnLeft();
		}
		else if (direction == 1)
		{
			turnRight();
		}
		else
		{
			turnStraight();
		}
	}


	controller.updateResponse(1.0 * READINGS[REAR_SENSOR]);
}
else
{

		
	if (abs(READINGS[RIGHT_SENSOR] - READINGS[LEFT_SENSOR]) < TURN_THRESHOLD)
	{
		turnStraight();
	}
	else if (READINGS[LEFT_SENSOR] > TURN_BOUNDARY
		&& READINGS[RIGHT_SENSOR] < (READINGS[LEFT_SENSOR] - TURN_THRESHOLD)
		&& abs(controller.state.output) > THROTTLE_THRESHOLD)
	{
		turnRight();
	}
	else if (READINGS[RIGHT_SENSOR] > TURN_BOUNDARY
		&& READINGS[LEFT_SENSOR] < (READINGS[RIGHT_SENSOR] - TURN_THRESHOLD)
		&& abs(controller.state.output) > THROTTLE_THRESHOLD)
	{
		turnLeft();
	}
	else
	{
		turnStraight();
	}

	controller.updateResponse(1.0 * READINGS[FWD_SENSOR]);
		
}

if (controller.state.output > 0)
{
	driveForward(controller.state.output);
}
else
{
	driveBackward(abs(controller.state.output));
}
```
###3. Stuck Detection

Now, how do we tell if we get stuck? There are smarter ways of doing it, but the way we implemented it is like this:

First, we keep a log of all the most recent wheel readings (1000 of them). Then, we check all the readings against the first one. If they're all the same, that means the wheel isn't spinning (e.g., the sensor isn't reading white and then black and then white and then black from the front wheel).
```cpp
wheel_reading = 0;
int first_reading = WHEEL_READINGS[0];
stopped = true;
for (int i = 1; i < WREADING_BUF_SIZE; i++)
{
	if (WHEEL_READINGS[i] > first_reading + WREADING_THRESHOLD
		|| WHEEL_READINGS[i] < first_reading - WREADING_THRESHOLD)
	{
		stopped = false;
		break;
	}
}
```
If we detect we're stopped, we'll reverse the direction. If the new direction happens to be backwards, we'll choose a random direction out of three to turn (so we don't get stuck again when we start moving forwards). 
```cpp
if (stopped)
{
	stopped = false;
	//if there's space to reverse, go backwards, otherwise go forward
	if (state == FORWARD)
	{
		state = BACKWARD;
	}
	else
	{
		state = FORWARD;
	}
	direction = rand() / 10922.0;
	controller.reverseControllerMode();

}
```

##Conclusion

Hopefully this writeup helps you in getting started on your very own autonomous car (big or small, we're excited to see either!). We'd love to see your creations, and hear about your ideas or potential tweaks. 

Feel free to fork this repository and use it as a starting point, or make your very own! 