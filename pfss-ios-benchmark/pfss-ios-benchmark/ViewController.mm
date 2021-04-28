//
//  ViewController.m
//  pfss-ios-benchmark
//
//  Created by Steve on 9/27/20.
//

#import "ViewController.h"
#import "pfss/standard_benchmark_runner.hpp"
#import <string>

@interface ViewController ()
{
    pfss::standard_benchmark_runner runner;
    IBOutlet UIButton *button;
    int tap_count;
    bool saved_to_file;
}
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

- (IBAction)onTouchDown:(id)sender {
    std::string const s1 = "Benchmark running, tap anywhere to check for completion";
    if (!runner.has_been_started()) {
        runner.start();
        tap_count = 0;
        saved_to_file = false;
        std::string const s = s1;
        [button setTitle:@(s.c_str()) forState:UIControlStateNormal];
    } else if (!runner.result_is_ready()) {
        ++tap_count;
        std::string const s = s1 + " (" + std::to_string(tap_count) + ")";
        [button setTitle:@(s.c_str()) forState:UIControlStateNormal];
    } else if (!saved_to_file) {
        NSString * result = [NSString stringWithCString:runner.get_result().c_str() encoding:NSUTF8StringEncoding];
        NSArray * dirs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString * dir = [dirs objectAtIndex:0];
        NSString * path = [dir stringByAppendingPathComponent:@"pfss-standard-benchmark.txt"];
        if ([result writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:NULL]) {
            saved_to_file = true;
            [button setTitle:@"Benchmark done, result saved to file, restart to benchmark again" forState:UIControlStateNormal];
        } else {
            [button setTitle:@"Benchmark done but an error occurred saving the result to file, tap anywhere to try again" forState:UIControlStateNormal];
        }
    }
}

@end
